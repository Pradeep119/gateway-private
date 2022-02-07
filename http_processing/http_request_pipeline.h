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

#include <boost/asio.hpp>

#include <core.h>

#include "../services/configurator/configurator_service.h"
#include "../generic_message_factory.h"
#include "filter_definitions.h"
#include "message_augmentor_definitions.h"
#include "validator_definitions.h"
#include "jwt_validation_defs.h"

namespace core = adl::axp::core;
namespace net = boost::asio;
namespace gateway = adl::axp::gateway;

namespace adl::axp::gateway::http {

using http_dispatcher_stage_t = core::stages::HttpDispatcherStage<
    MessageFactory<core::event_processing::messages::HttpResponseMessage>,
    MessageFactory<core::stages::details::http_dispatcher::HttpDroppedRequestMessage>>;

/**
 * a pipeline is similar to a stage but internally has many stages.
 * To the outside it gives same interface as a stage.
 */
class HttpRequestProcessingPipeline : public core::event_processing::details::BaseStage {
 public:
  HttpRequestProcessingPipeline(std::string_view name, net::io_context &io_context) :
      core::event_processing::details::BaseStage(std::string(name)),
      _http_dispatcher("http_dispatcher", 50, io_context, _http_response_message_factory, _dropped_message_factory) {

    register_sink("message_in", _router.get_sink("message_in"));

    register_source("throttling_allowed_out", &_throttling_allowed_out);
    register_source("throttling_suppressed_out", &_throttling_suppressed_out);
    register_source("throttle_error_out", throttling_enforcer.get_source("throttle_error_out"));
    register_source("response_out", &_response_out);
    register_source("dropped_out", &_dropped_out);
  }

  void assemble();
  void start() {}

  core::services::store_services::KeyValueStore &get_key_value_store() {
    return _store;
  }

  core::services::store_services::KeyComplexValueStore<core::stages::http::validation::RequestValidator> &get_validation_spec_store() {
    return _validation_spec_store;
  }

 private:
  // import various stuff to be used in stages
  static gateway::http::MessageAugmentorDefs _message_aug_defs;
  static gateway::http::WhiteBlackListFilterDefs _filter_defs;
  axp::gateway::http::HttpHeaderValidatorDefs _http_validation_defs;
  axp::gateway::JwtValidation::JwtValidationDefs _jwt_validation_defs;
  axp::gateway::JwtValidation::RSAKeys _rsa_keys;

  // non stage data members
  core::services::store_services::KeyValueStore _store;
  core::services::store_services::KeyComplexValueStore<core::stages::http::validation::RequestValidator>
      _validation_spec_store;

  // factories
  MessageFactory<core::event_processing::messages::HttpResponseMessage> _http_response_message_factory;
  MessageFactory<core::stages::details::http_dispatcher::HttpDroppedRequestMessage> _dropped_message_factory;

  // stages
  http_dispatcher_stage_t _http_dispatcher;

  core::stages::FourWayRouterStage<core::extensions::compile_time_hash(
      "adl::axp::core::event_processing::base_impl::HttpRequestMessage"),
                                   core::extensions::compile_time_hash(
                                       "adl::axp::core::event_processing::messages::TaskExecutionMessage"),
                                   core::extensions::compile_time_hash(
                                       "adl::axp::core::stages::throttling::ThrottleStateMessage")>
      _router{"router 1"};
  core::stages::message_augmentor::MessageAugmentorStage
      _message_augmentor{"message aug 1", _message_aug_defs._augmentors.begin(), _message_aug_defs._augmentors.end()};
  core::stages::misc::DelayStage _delay{"delay", 100};

  core::stages::http::validation::HttpRequestValidationStage
      _http_request_validator_stage{"http request validator stage", _validation_spec_store,
                                    _http_validation_defs._http_validator_key_field_combination,
                                    _http_validation_defs._header_field_group,
                                    _http_validation_defs._path_field_group,
                                    _http_validation_defs._query_field_group};

  core::stages::load_balancers::RoundRobinLoadBalancerStage
      _end_point_load_balancer{"lb stage", {"backendpools", "api"}, _store};

  core::stages::Jwt::JwtDecoderStage _jwt_decoder{"jwt_decoder", "access_token"};

  core::stages::jwt_token::JwtValidationStage
      _jwt_validator{"jwt_validator", _jwt_validation_defs._token_field, "token_info", _rsa_keys._rsa_pub_key_1};

  core::stages::misc::BroadcastingState<2> _broadcaster{"broadcaster"};
  core::stages::throttling::ThrottlingEnforcerStage throttling_enforcer{"http message throttler", {"route_key_1"}};

  core::stages::filter_stage::WhitelistingStage _whitelister
      {"whitelist stage sender ip", _store, _filter_defs._wl_field_combinations};
  core::stages::filter_stage::BlacklistingStage _blacklister
      {"blacklist stage sender ip", _store, _filter_defs._bl_field_combinations};

  // sinks and sources of the pipeline
  core::event_processing::details::BaseSource _throttling_allowed_out;
  core::event_processing::details::BaseSource _throttling_suppressed_out;
  core::event_processing::details::BaseSource _response_out;
  core::event_processing::details::BaseSource _dropped_out;

};

}
