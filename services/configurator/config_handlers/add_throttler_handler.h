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
 * @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-09-30.
 */

#pragma once

#include <core.h>
#include "cxxopts_configuration_command_handler_base.h"
#include "../errors.h"

namespace core = adl::axp::core;

namespace adl::axp::gateway::services::configurator {

template<typename config_message_t>
class AddUpdateThrottlerConfigHandler : public CXXOptsConfigurationCommandHandlerBase {
 public:
  ConfigCommandResult handle(int &size, char **&tokens) noexcept(false) override {
    cxxopts::Options options("add_throttler", "Add a new throttler to the throttling stage");
    options.add_options()
        ("k,key", "Unique key for this throttler", cxxopts::value<std::string>())
        ("t,type", "Type of the throttler [sliding_window_thread_safe, sliding_window]", cxxopts::value<std::string>())
        ("q,queuing",
         "if true, throttled messages are added to a queue and reconsidered later. If false, throttled messages are not forwarded",
         cxxopts::value<bool>())
        ("w,window_duration",
         "Throttling window duration",
         cxxopts::value<size_t>()) //TODO: handle different resolutions
        ("c,allowed_count",
         "Allowed count in the given duration",
         cxxopts::value<size_t>()); //TODO: handle different resolutions

    try {
      auto result = options.parse(size, tokens);

      auto *add_throttler_config_message = new config_message_t(
          result["key"].as<std::string>(),
          result["type"].as<std::string>(),
          false, // result["queuing"].as < bool >() , TODO: fix this
          result["window_duration"].as<size_t>(),
          result["allowed_count"].as<size_t>()
      );

      auto apply_function = [add_throttler_config_message](core::event_processing::ISink *throttler) {
        throttler->on_message(add_throttler_config_message);
        add_throttler_config_message->get_future().get(); // will throw if there were issues processing message
      };

      return apply<std::function<void(core::event_processing::ISink *)>,
                   targets_t::iterator>(apply_function, _throttlers.begin(), _throttlers.end());

    } catch (const cxxopts::OptionSpecException &ex) {
      throw ConfigCommandParseException("add_throttler", ex.what());
    } catch (const cxxopts::OptionParseException &ex) {
      throw ConfigCommandParseException("add_throttler", ex.what());
    }
  }

  std::string_view get_usage_string() const noexcept override {
    return "";
  }

  class AddUpdateThrottlerConfigHandlerBuilder {
   public:
    AddUpdateThrottlerConfigHandlerBuilder &add_throttler_config_sink(core::event_processing::ISink &throttler_config_sink,
                                                                      core::event_processing::ISink &sink) {
      _handler->_throttlers.emplace_back(&throttler_config_sink, &sink);
      return *this;
    }

    AddUpdateThrottlerConfigHandler *build() {
      return _handler;
    }

   private:
    AddUpdateThrottlerConfigHandler<config_message_t>
        *_handler = {new AddUpdateThrottlerConfigHandler<config_message_t>()};
  };

  static AddUpdateThrottlerConfigHandlerBuilder *new_builder() {
    return new AddUpdateThrottlerConfigHandlerBuilder();
  }

 private:
  using targets_t = std::list<std::pair<core::event_processing::ISink *, core::event_processing::ISink *>>;
  targets_t _throttlers;

  friend class AddUpdateThrottlerConfigHandlerBuilder;
};

using AddThrottlerConfigHandler = AddUpdateThrottlerConfigHandler<core::stages::throttling::AddThrottlerConfigMessage>;
using UpdateThrottlerConfigHandler = AddUpdateThrottlerConfigHandler<core::stages::throttling::UpdateThrottlerConfigMessage>;

}