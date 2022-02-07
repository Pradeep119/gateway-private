/*
 * © Copyrights 2021 Axiata Digital Labs Pvt Ltd.
 * All Rights Reserved.
 *
 * These material are unpublished, proprietary, confidential source
 * code of Axiata Digital Labs Pvt Ltd (ADL) and constitute a TRADE
 * SECRET of ADL.
 *
 * ADL retains all title to and intellectual property rights in these
 * materials.
 *
 *
 * @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-09-23.
 */

#include "http_request_pipeline.h"

namespace core = adl::axp::core;
namespace net = boost::asio;
namespace gateway = adl::axp::gateway;

namespace adl::axp::gateway::http {

namespace {

class Stamper : public core::event_processing::ISink {
 public:
  Stamper(std::string_view id, core::event_processing::ISink *target) : _id(id), _target(target) {}

  void on_message(core::event_processing::IMessage *message) noexcept override {
    auto *dyna = dynamic_cast<core::event_processing::DynamicMessage *>(message);
    if (dyna != nullptr) {
      auto to_millies = [](const timespec &time_stamp) {
        return time_stamp.tv_nsec / 1'000'000 + time_stamp.tv_sec * 1000;
      };
      timespec time_stamp;
      clock_gettime(CLOCK_REALTIME, &time_stamp);
      int64_t time_stamp_millies = to_millies(time_stamp);

      dyna->create_typed_group<int64_t>("stamps");
      dyna->set_typed_field<int64_t>("stamps", _id, time_stamp_millies);
      _target->on_message(dyna);
    } else {
      _target->on_message(message);
    }
  }

  const std::string _id;
  core::event_processing::ISink *_target;
};

void connect(core::event_processing::IStage &source_stage,
             const char *source_name,
             core::event_processing::IStage &sink_stage,
             const char *sink_name) {
  std::stringstream ss;
  ss << source_stage.get_name() << " <" << source_name << "> ==> " << sink_stage.get_name() << " <" << sink_name << ">";
  source_stage.get_source(source_name)->set_sink(new Stamper(ss.str(), sink_stage.get_sink(sink_name)));
}

void connect(core::event_processing::IStage &source_stage,
             const char *source_name,
             core::event_processing::ISink &sink, std::string_view name_for_sink) {
  std::stringstream ss;
  ss << source_stage.get_name() << " <" << source_name << "> ==> " << " <" << name_for_sink << ">";
  source_stage.get_source(source_name)->set_sink(new Stamper(std::string(source_name) + "--> []", &sink));
}
}

gateway::http::MessageAugmentorDefs HttpRequestProcessingPipeline::_message_aug_defs;
gateway::http::WhiteBlackListFilterDefs HttpRequestProcessingPipeline::_filter_defs;

void HttpRequestProcessingPipeline::assemble() {

  connect(_router, "source_1", _message_augmentor, "message_in");
  connect(_router, "source_2", *new core::sinks::TaskExecutionSink(), "task exec 1");
  connect(_router, "source_3", throttling_enforcer, "throttle_state_in");
  connect(_router, "remaining_source", *new core::stages::misc::NoOpSink(), "No ops 1");

  connect(_message_augmentor, "message_out", _http_request_validator_stage, "in");

  connect(_http_request_validator_stage, "valid_requests_out", _whitelister, "message_in");
  connect(_whitelister, "out", _blacklister, "message_in");
  connect(_whitelister, "exception_out", *new core::stages::misc::NoOpSink(), "No ops 2");
  connect(_whitelister, "filtered", *new core::stages::misc::NoOpSink(), "No ops 2");

  connect(_blacklister, "filtered", *new core::stages::misc::NoOpSink(), "No ops 3");
  connect(_blacklister, "exception_out", *new core::stages::misc::NoOpSink(), "No ops 4");
  connect(_blacklister, "out", _jwt_validator, "message_in");

  connect(_jwt_validator, "valid_out", throttling_enforcer, "message_in");
  connect(_jwt_validator, "invalid_out", *new core::stages::misc::NoOpSink(), "No ops 5");
  connect(_jwt_validator, "expired_out", *new core::stages::misc::NoOpSink(), "No ops 6");
  connect(_jwt_validator, "non_jwt_out", *new core::stages::misc::NoOpSink(), "No ops 7");

  connect(throttling_enforcer, "allowed_out", _broadcaster, "in");
  connect(throttling_enforcer, "throttled_out", *_throttling_suppressed_out.get_sink(), "throttling allowed 2");
  connect(_broadcaster, "out-0", _end_point_load_balancer, "in");
  connect(_broadcaster, "out-1", *_throttling_allowed_out.get_sink(), "throttling allowed 1");

  connect(_end_point_load_balancer, "out", _http_dispatcher, "http_request_in");

  connect(_http_dispatcher, "dropped_requests_out", *_dropped_out.get_sink(), "dropped 1");
  connect(_http_dispatcher, "http_response_out", *_response_out.get_sink(), "dropped 2");

  connect(_jwt_decoder, "error_decoding_out", *new core::stages::misc::NoOpSink(), "no ops 10");
  connect(_http_request_validator_stage, "invalid_requests_out", *new core::stages::misc::NoOpSink(), "no ops 11");
}

}
