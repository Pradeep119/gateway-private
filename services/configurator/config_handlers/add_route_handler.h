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
 * @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-09-16.
 */

#pragma once

#include <core.h>
#include "cxxopts_configuration_command_handler_base.h"
#include "../errors.h"
#include "common.h"

namespace core = adl::axp::core;

namespace adl::axp::gateway::services::configurator {

class AddRouteConfigHandler : public CXXOptsConfigurationCommandHandlerBase {
  static core::foundation::HttpMethods to_enum(std::string_view method) {
    return core::foundation::HttpMethods::GET; //TODO:
  }

 public:

  ConfigCommandResult handle(int &size, char **&tokens) noexcept(false) override {
    cxxopts::Options options("add_route", "Adds a new http route");
    options.add_options()
        ("route_key_1", "User key 1", cxxopts::value<std::string>())
        ("route_key_2", "User key 2", cxxopts::value<std::string>())
        ("r,resource", "Resource path", cxxopts::value<std::string>())
        ("m,method", "Http method", cxxopts::value<std::string>()->default_value("GET"));

    try {
      auto result = options.parse(size, tokens);

      const auto &route_key_1 = result["route_key_1"].as<std::string>();
      const auto &route_key_2 = result["route_key_2"].as<std::string>();
      const auto &resource = result["resource"].as<std::string>();
      const auto &method = result["method"].as<std::string>();

      auto apply_function = [route_key_1, route_key_2, resource, method](core::event_processing::ISink *stage) -> void {
        auto *route_config_message =
            new core::stages::http_server_stage::AddRouteConfigMessage(route_key_1,
                                                                       route_key_2,
                                                                       resource,
                                                                       to_enum(method));
        stage->on_message(route_config_message); //TODO: use future to get any exceptions
        route_config_message->get_future().get(); // will throw if there were issues processing message
      };

      return apply<std::function<void(core::event_processing::ISink *)>,
                   targets_t::iterator>(apply_function, _http_server_stages.begin(), _http_server_stages.end());

    } catch (const cxxopts::OptionSpecException &ex) {
      throw ConfigCommandParseException("add_throttler", ex.what());
    } catch (const cxxopts::OptionParseException &ex) {
      throw ConfigCommandParseException("add_throttler", ex.what());
    }
  }

  std::string_view get_usage_string() const noexcept override {
    return "";
  }

  class AddRouteConfigHandlerBuilder {
   public:
    AddRouteConfigHandlerBuilder &add_http_server_config_sink(core::event_processing::ISink &http_server_stage,
                                                              core::event_processing::ISink &sink) {
      _handler->_http_server_stages.emplace_back(&http_server_stage, &sink);
      return *this;
    }
    AddRouteConfigHandler *build() {
      return _handler;
    }

   private:
    AddRouteConfigHandler *_handler = {new AddRouteConfigHandler()};
  };

  static AddRouteConfigHandlerBuilder *new_builder() {
    return new AddRouteConfigHandlerBuilder();
  }
 private:
  using targets_t = std::list<std::pair<core::event_processing::ISink *, core::event_processing::ISink *>>;
  targets_t _http_server_stages;

  using targets_v = std::list<std::pair<core::services::store_services::KeyValueStore *,
                                        core::event_processing::ISink *>>;
  targets_v _stores;
  friend class AddRouteConfigHandlerBuilder;
};

}