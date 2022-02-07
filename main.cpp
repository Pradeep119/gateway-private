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
 * @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-08-02.
 */

#include <iostream>
#include <array>

#include <time.h>

#include <uWebSockets/App.h>
#include <boost/asio.hpp>
#include <boost/pool/object_pool.hpp>

#include <core.h>

#include "stages/configurator/configurator_tcp_server_stage.h"
#include "stages/unwrap_stage.h"
#include "services/configurator/configurator_service.h"
#include "services/observability/observability_service.h"
#include "http_processing/http_request_pipeline.h"
#include "generic_message_factory.h"

namespace core = adl::axp::core;
namespace net = boost::asio;
namespace gateway = adl::axp::gateway;

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
}

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

namespace {

class MetaHttpRequestMessage : public core::event_processing::messages::HttpRequestMessage {
 public:
  void set_http_server_response_sink(core::event_processing::ISink *sink) {
    _http_server_response_sink = sink;
  }

  core::event_processing::ISink *get_http_server_response_sink() {
    return _http_server_response_sink;
  }

 private:
  core::event_processing::ISink *_http_server_response_sink;
};

class HttpRequestMessageFactory {
 public:
  void set_http_server_response_sink(core::event_processing::ISink *sink) {
    _http_server_response_sink = sink;
  }

  core::event_processing::messages::HttpRequestMessage *create() {
    auto *message = new MetaHttpRequestMessage();
    message->set_http_server_response_sink(_http_server_response_sink);
    return message;
  }

 private:
  core::event_processing::ISink *_http_server_response_sink;
};

/**
 * Thread safe - yes
 */
class HttpResponseRouterSink : public core::event_processing::ISink {
 public:
  virtual void on_message(core::event_processing::IMessage *message) noexcept {
    auto *http_response_message = static_cast<core::event_processing::messages::HttpResponseMessage *>(message);
    auto &meta_message = static_cast<MetaHttpRequestMessage &>(http_response_message->get_request_message());
    meta_message.get_http_server_response_sink()->on_message(message);
  }
};

using http_server_stage_t = core::stages::http_server_stage::HttpServerStage<HttpRequestMessageFactory>;
uWS::Loop *uws_event_loop_1 = nullptr; //TODO: clean this up
uWS::Loop *uws_event_loop_2 = nullptr; //TODO: clean this up
uWS::Loop *uws_event_loop_3 = nullptr; //TODO: clean this up

}

/**
 * Runs the boost beast event loop
 * @param iocThe underlying IO context
 */
void run_beast_loop(net::io_context &ioc) {
  try {
    ioc.run();
  } catch (const std::exception &ex) {
    std::cout << "error while running " << ex.what() << std::endl;
  }
  std::cout << "exiting beast loop" << std::endl;
}

/**
 * Run the uSocket loop.
 * @param server_stage stage(s) that use this loop
 */
void run_usocket_loop(http_server_stage_t &server_stage, uWS::Loop *&loop) {
  server_stage.start(); // start has to be called from the same thread that runs the loop
  loop = uWS::Loop::get();
  loop->run();
}

class UWSLoopPostingSink : public core::event_processing::ISink {
 public:
  explicit UWSLoopPostingSink(uWS::Loop *&loop, core::event_processing::ISink &target_sink) :
      _loop(loop),
      _target_sink(target_sink) {
  }

  void on_message(core::event_processing::IMessage *message) noexcept override {
    _loop->defer([this, message]() {
      _target_sink.on_message(message);
    });
  }

 private:
  uWS::Loop *&_loop;
  core::event_processing::ISink &_target_sink;
};

class HttpResponder : public core::event_processing::details::BaseStage,
                      core::event_processing::ISink {
 public:
  HttpResponder() : core::event_processing::details::BaseStage("responder") {
    register_sink("in", this);
    register_source("out", &_out);
  }

  void on_message(core::event_processing::IMessage *message) noexcept override {
    auto *response_message = new core::event_processing::messages::HttpResponseMessage();
    response_message->set_body("");
    response_message->set_return_code(500);
    response_message->set_request_message(static_cast<core::event_processing::messages::HttpRequestMessage &>(*message));
    _out.get_sink()->on_message(response_message);
  }

 private:
  core::event_processing::details::BaseSource _out;
};

class TimerMessage : public core::event_processing::IMessage {
 public:
  size_t message_type() const noexcept override {
    return core::extensions::compile_time_hash("TimerMessage");
  }
};

class TimerEventSource : public core::event_processing::details::BaseSource,
                         public core::event_processing::IRunnable {
 public:
  TimerEventSource(size_t period) : _period(period) {}

  int run(void *data) noexcept override {
    std::chrono::microseconds us{_period};
    while (true) {
      std::this_thread::sleep_for(us);
      adl::axp::core::event_processing::details::BaseSource::get_sink()->on_message(new TimerMessage());
    }
  }
  int stop() noexcept override {
    return 0;
  }

 private:
  const size_t _period;
};

class ConfigBridge : public core::event_processing::details::BaseStage,
                     core::event_processing::ISink {
 public:
  ConfigBridge(adl::axp::gateway::services::configurator::ConfiguratorService &config_service) :
      core::event_processing::details::BaseStage("config bridge"),
      _config_service(config_service) {
    register_sink("config_in", this);
    register_source("result_out", &_out);
  }

  void on_message(core::event_processing::IMessage *message) noexcept override {
    auto config_message = static_cast<adl::axp::gateway::services::configurator::ConfigurationMessage *>(message);
    const auto &result = _config_service.execute_config_message(config_message) + "\r";
    auto result_message = new core::stages::TcpResponseMessage(config_message, result);
    _out.get_sink()->on_message(result_message);
  }

 private:
  adl::axp::gateway::services::configurator::ConfiguratorService &_config_service;

  core::event_processing::details::BaseSource _out;
};

/**
 * Entry point of the app
 * @return
 */
int main(int argc, char **argv) {

  if (argc < 3) {
    std::cerr << "Usage - gateway <port offset> <throttle multicast port>" << std::endl;
    exit(-1);
  }

  const int port_offset = std::stoi(argv[1]);
  const int throttle_multicast_port = std::stoi(argv[2]);

  bool stop;

  // common vars
  net::io_context io_context_1;
  net::io_context io_context_2;
  HttpRequestMessageFactory http_request_message_factory_1;
  HttpRequestMessageFactory http_request_message_factory_2;
  HttpRequestMessageFactory http_request_message_factory_3;
  boost::asio::executor_work_guard<boost::asio::io_context::executor_type>
      work_1 = boost::asio::make_work_guard(io_context_1);
  boost::asio::executor_work_guard<boost::asio::io_context::executor_type>
      work_2 = boost::asio::make_work_guard(io_context_2);

  http_server_stage_t https_server_1("https_server_1", "./key.pem",
                                     "./cert.pem",
                                     "1234", port_offset, http_request_message_factory_1);
  http_server_stage_t https_server_2("https_server_2", "./key.pem",
                                     "./cert.pem",
                                     "1234", port_offset + 1, http_request_message_factory_2);
  http_server_stage_t https_server_3("https_server_3", "./key.pem",
                                     "./cert.pem",
                                     "1234", port_offset + 2, http_request_message_factory_3);

  struct DummyAssembler {};
  core::stages::UdpServerStage<DummyAssembler>
      udp_throttle_ingress("throttle_info_server", io_context_1, "0.0.0.0", throttle_multicast_port, "239.255.0.1");

  core::stages::load_balancers::InProcessRoundRobinLoadBalancerStage<2>
      load_balancer("http request pipelines load balancer");

  core::stages::MPMCQueue queue_1("http req queue 1", 100);
  core::stages::MPMCQueue queue_2("http req queue 2", 100);
  core::stages::MPMCQueue drop_copy_queue("drop_copy_queue", 100);

  gateway::http::HttpRequestProcessingPipeline http_pipeline_1("http processing pipeline 1", io_context_1);
  gateway::http::HttpRequestProcessingPipeline http_pipeline_2("http processing pipeline 2", io_context_2);

  core::stages::MPMCQueue http_res_queue_1("http res queue 1", 100);
  core::stages::MPMCQueue http_res_queue_2("http res queue 2", 100);

  HttpResponseRouterSink response_router;
  HttpResponder error_responder;
  core::stages::misc::BroadcastingState<2> throttling_suppressed_bc("throttling_suppressed_bc");

  core::stages::throttling::ThrottleStateBuilderStage
      throttle_state_builder("throttle_state_builder", {"route_key_1"}, "request_time", 5);
  core::stages::misc::BroadcastingState<2> throttler_state_message_boradcaster("throttler_state_bc");
  core::stages::ThreeWayRouterStage<core::extensions::compile_time_hash(
      "adl::axp::core::event_processing::messages::TaskExecutionMessage"),
                                    core::extensions::compile_time_hash("TimerMessage")>
      router{"router stage"};

  TimerEventSource timer(10000);
  core::stages::misc::DelayStage resubmit_delay_1{"delay stage 1", 1000000};
  core::stages::misc::DelayStage resubmit_delay_2{"delay stage 2", 1000000};

  // connect
  connect(https_server_1, "http_request_out", load_balancer, "message_in");
  connect(https_server_2, "http_request_out", load_balancer, "message_in");
  connect(https_server_3, "http_request_out", load_balancer, "message_in");
  connect(load_balancer, "message_out_0", queue_1, "message_in");
  connect(load_balancer, "message_out_1", queue_2, "message_in");
  connect(queue_1, "message_out", http_pipeline_1, "message_in");
  connect(queue_2, "message_out", http_pipeline_2, "message_in");
  connect(http_pipeline_1, "response_out", http_res_queue_1, "message_in");
  connect(http_pipeline_2, "response_out", http_res_queue_2, "message_in");

  connect(http_pipeline_1, "dropped_out", resubmit_delay_1, "in");
  connect(http_pipeline_2, "dropped_out", resubmit_delay_2, "in");
  connect(resubmit_delay_1, "out", queue_1, "message_in");
  connect(resubmit_delay_2, "out", queue_2, "message_in");

  connect(http_res_queue_1, "message_out", response_router, "response router");
  connect(http_res_queue_2, "message_out", response_router, "response router");

  connect(throttle_state_builder, "state_out", throttler_state_message_boradcaster, "in");
  connect(throttler_state_message_boradcaster, "out-0", queue_1, "message_in");
  connect(throttler_state_message_boradcaster, "out-1", queue_2, "message_in");
  connect(throttle_state_builder,
          "error_out",
          *new core::sinks::PrintingSink("throttle state building error"),
          "printing 1");

//  connect(throttler, "throttled_out", *new core::sinks::PrintingSink("throttled a message"));
  connect(http_pipeline_1, "throttling_allowed_out", drop_copy_queue, "message_in");
  connect(http_pipeline_2, "throttling_allowed_out", drop_copy_queue, "message_in");

  connect(http_pipeline_1, "throttling_suppressed_out", throttling_suppressed_bc, "in");
  connect(http_pipeline_2, "throttling_suppressed_out", throttling_suppressed_bc, "in");

  connect(throttling_suppressed_bc, "out-0", error_responder, "in");
  connect(throttling_suppressed_bc, "out-1", drop_copy_queue, "message_in");

  connect(http_pipeline_1, "throttle_error_out", error_responder, "in");
  connect(error_responder, "out", response_router, "response router");

  timer.set_sink(drop_copy_queue.get_sink("message_in"));
  connect(drop_copy_queue, "message_out", router, "message_in");
  connect(router, "source_1", *new core::sinks::TaskExecutionSink(), "task exec 2");
  connect(router, "source_2", throttle_state_builder, "flush_in");
  connect(router, "remaining_source", throttle_state_builder, "message_in");

  connect(queue_1, "buffer_full_out", error_responder, "in");
  connect(queue_2, "buffer_full_out", error_responder, "in");
  connect(drop_copy_queue, "buffer_full_out", *new core::sinks::PrintingSink("drop_copy_queue buffer full"), "print 2");
  connect(http_res_queue_1,
          "buffer_full_out",
          *new core::sinks::PrintingSink("http_res_queue_1 buffer full"),
          "print 3");
  connect(http_res_queue_2,
          "buffer_full_out",
          *new core::sinks::PrintingSink("http_res_queue_2 buffer full"),
          "print 4");

  UWSLoopPostingSink response_posting_sink_1(uws_event_loop_1, *https_server_1.get_sink("http_response_in"));
  UWSLoopPostingSink response_posting_sink_2(uws_event_loop_2, *https_server_2.get_sink("http_response_in"));
  UWSLoopPostingSink response_posting_sink_3(uws_event_loop_3, *https_server_3.get_sink("http_response_in"));

  http_request_message_factory_1.set_http_server_response_sink(&response_posting_sink_1);
  http_request_message_factory_2.set_http_server_response_sink(&response_posting_sink_2);
  http_request_message_factory_3.set_http_server_response_sink(&response_posting_sink_3);

  // other
  gateway::stages::configurator::ConfiguratorTcpServerStage
      config_tcp_server("config tcp server", io_context_2, port_offset + 1000);
  UWSLoopPostingSink configuration_posting_sink_1(uws_event_loop_1, *new core::sinks::TaskExecutionSink());
  UWSLoopPostingSink configuration_posting_sink_2(uws_event_loop_2, *new core::sinks::TaskExecutionSink());
  UWSLoopPostingSink configuration_posting_sink_3(uws_event_loop_3, *new core::sinks::TaskExecutionSink());

  // TODO: better way for below?

  std::list<core::event_processing::IStage *> all_stages = {
      &https_server_1, &https_server_2, &https_server_3, &queue_1, &queue_2, &http_pipeline_1, &http_pipeline_2,
      &http_res_queue_1, &http_res_queue_2, &drop_copy_queue
  };

  // services
  adl::axp::gateway::services::configurator::ConfiguratorService *configurator_service =
      adl::axp::gateway::services::configurator::ConfiguratorService::newBuilder()
          ->add_http_server_stage_config_sink(https_server_1.get_sink("configuration_in"),
                                              &configuration_posting_sink_1)
          ->add_http_server_stage_config_sink(https_server_2.get_sink("configuration_in"),
                                              &configuration_posting_sink_2)
          ->add_http_server_stage_config_sink(https_server_3.get_sink("configuration_in"),
                                              &configuration_posting_sink_3) //TODO: we really dont have to pass first param.
          ->add_key_value_store(&http_pipeline_1.get_key_value_store(), queue_1.get_sink("message_in"))
          ->add_key_value_store(&http_pipeline_2.get_key_value_store(), queue_2.get_sink("message_in"))
          ->add_validation_spec_store(&http_pipeline_1.get_validation_spec_store(), queue_1.get_sink("message_in"))
          ->add_validation_spec_store(&http_pipeline_2.get_validation_spec_store(), queue_2.get_sink("message_in"))
          ->add_throttler_stage_config_target(throttle_state_builder.get_sink("config_in"),
                                              drop_copy_queue.get_sink("message_in"))
          ->build();

  adl::axp::gateway::services::observability::ObservabilityService observability_service;

  // config wiring
  ConfigBridge config_bridge{*configurator_service};
  connect(config_tcp_server, "tcp_request_out", config_bridge, "config_in");
  connect(config_bridge, "result_out", config_tcp_server, "tcp_response_in");

  // starting
  configurator_service->start();
  observability_service.start();
  queue_1.start();
  queue_2.start();
  drop_copy_queue.start();
  http_pipeline_1.assemble();
  http_pipeline_2.assemble();
  http_pipeline_1.start();
  http_pipeline_2.start();
  config_tcp_server.start();
  udp_throttle_ingress.start();

  // running
  std::list<std::thread *> run_threads;
  for (auto *stage : all_stages) {
    core::event_processing::IRunnable *runnable = dynamic_cast<core::event_processing::IRunnable *>(stage);
    if (nullptr != runnable) {
      std::thread *thread = new std::thread(&core::event_processing::IRunnable::run, runnable, nullptr);
      run_threads.push_back(thread);
    }
  }

  // common event loops
  std::thread usocket_runner_1(run_usocket_loop, std::ref(https_server_1), std::ref(uws_event_loop_1));
  std::thread usocket_runner_2(run_usocket_loop, std::ref(https_server_2), std::ref(uws_event_loop_2));
  std::thread usocket_runner_3(run_usocket_loop, std::ref(https_server_3), std::ref(uws_event_loop_3));
  std::thread beast_runner_1(run_beast_loop, std::ref(io_context_1));
  std::thread beast_runner_2(run_beast_loop, std::ref(io_context_2));
  timer.run(nullptr);

  // shutting down
//  usocket_runner.join();
//  beast_runner.join();

  for (auto *thread : run_threads) {
    thread->join();
  }

  return 0;
}