/*
 *
 *   © Copyrights 2021 Axiata Digital Labs Pvt Ltd.
 *   All Rights Reserved.
 *
 *   These material are unpublished, proprietary, confidential source
 *   code of Axiata Digital Labs Pvt Ltd (ADL) and constitute a TRADE
 *   SECRET of ADL.
 *
 *   ADL retains all title to and intellectual property rights in these
 *   materials.
 *
 *
 *   @author Janith Priyankara (janith.priyankara@axiatadigitallabs.com) on 10/11/21.
 *
 */

#pragma once

#include <core.h>
#include "cxxopts_configuration_command_handler_base.h"
#include "../errors.h"

namespace core = adl::axp::core;

namespace adl::axp::gateway::services::configurator {

class AddToSetConfigHandler : public CXXOptsConfigurationCommandHandlerBase {
 public:
  ConfigCommandResult handle(int &size, char **&tokens) noexcept(false) override {
    cxxopts::Options options("add_to_set", "Adds a new item to a given set with a specified key");
    options.add_options()
        ("k,key", "Key", cxxopts::value<std::string>())
        ("t,value_type",
         "Type of the value [int16, uint16, int32, uint32, int64, uint64, string, float, double, bool]",
         cxxopts::value<std::string>())
        ("v,value", "Value", cxxopts::value<std::string>());

    try {
      auto result = options.parse(size, tokens);

      const auto &value_type = result["value_type"].as<std::string>();
      const auto &key = result["key"].as<std::string>();
      const auto &value = result["value"].as<std::string>();

      if (value_type == "int32") {

        auto apply_function = [=](core::services::store_services::KeyValueStore *store) {
          store->add_to_set<int32_t>(key, std::stoi(value));
        };

        return apply<std::function<void(core::services::store_services::KeyValueStore*)>, targets_t::iterator>(
            apply_function,
            _stores.begin(),
            _stores.end());

      } else if (value_type == "string") {
        auto apply_function = [=](core::services::store_services::KeyValueStore *store) {
          store->add_to_set<std::string>(key, value);
        };

        return apply<std::function<void(core::services::store_services::KeyValueStore*)>, targets_t::iterator>(
            apply_function,
            _stores.begin(),
            _stores.end());
      } else {
        throw ConfigCommandParseException("add_to_set", std::string("Unknown value_type specified - ") + value_type);
      }
    } catch(const cxxopts::OptionSpecException &ex) {
      throw ConfigCommandParseException("add_throttler", ex.what());
    } catch(const cxxopts::OptionParseException &ex) {
      throw ConfigCommandParseException("add_throttler", ex.what());
    }

    return ConfigCommandResult{0, "", ""};
  }

  std::string_view get_usage_string() const noexcept override {
    return "";
  }

  class AddToSetConfigHandlerBuilder {
   public:
    AddToSetConfigHandlerBuilder &add_key_value_store(core::services::store_services::KeyValueStore &store,
                                                      core::event_processing::ISink &sink) {
      _handler->_stores.emplace_back(&store, &sink);
      return *this;
    }

    AddToSetConfigHandler *build() {
      return _handler;
    }

   private:
    AddToSetConfigHandler *_handler = {new AddToSetConfigHandler()};
  };

  static AddToSetConfigHandlerBuilder *new_builder() {
    return new AddToSetConfigHandlerBuilder();
  }

 private:
  using targets_t = std::list<std::pair<core::services::store_services::KeyValueStore *, core::event_processing::ISink *>>;
  targets_t _stores;

  friend class AddRouteConfigHandlerBuilder;
};

}