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
#include "configuration_message.h"
#include "configuration_command_handler.h"
#include "config_handlers/add_route_handler.h"
#include "config_handlers/add_key_value_handler.h"
#include "config_handlers/add_to_list_handler.h"
#include "config_handlers/add_http_validation_handler.h"
#include "config_handlers/add_throttler_handler.h"
#include "config_handlers/add_to_set_handler.h"
#include "errors.h"

namespace core = adl::axp::core;

namespace adl::axp::gateway::services::configurator {

/**
 * Configuration service.
 * Thread safe - no
 *
 * The configuration service is responsible for receiving configuration commands and executing them
 * appropriately to achieve required functionality. It communicates with various configurable stages and services.
 * Each service/stage is configured by posting configuration logic as a TaskExecution message. Each
 * configurable instance (service or stage) should have its own sink object which can be used to send
 * TaskExecution messages. These sinks should appropriately route these messages so that thread safety
 * of services and stages are not violated.
 *
 * In usual cases there are more than one instance of a given service or stage that must be configured
 * identically. This service accepts multiple such instances with posting sinks for each service/stage
 * type. They are configured identically in sequential order. No assumptions should be made regarding the
 * order of execution.
 *
 * This service is a sink too to receive config messages
 */
class ConfiguratorService {
 private:
  ConfiguratorService() = default;

 public:
  void start() {

    // install handlers

    {
      auto *handler_builder = AddKeyValueConfigHandler::newBuilder();
      std::ranges::for_each(_key_value_stores.begin(), _key_value_stores.end(), [handler_builder](auto &pair) {
        handler_builder->add_key_value_store(*pair.first, *pair.second);
      });
      _handlers.emplace("add_key_value", handler_builder->build());
    }
    {
      auto *handler_builder = AddToListConfigHandler::new_builder();
      std::ranges::for_each(_key_value_stores.begin(), _key_value_stores.end(), [handler_builder](auto &pair) {
        handler_builder->add_key_value_store(*pair.first, *pair.second);
      });
      _handlers.emplace("add_to_list", handler_builder->build());
    }
    {
      auto *handler_builder = AddToSetConfigHandler::new_builder();
      std::ranges::for_each(_key_value_stores.begin(), _key_value_stores.end(), [handler_builder](auto &pair) {
        handler_builder->add_key_value_store(*pair.first, *pair.second);
      });
      _handlers.emplace("add_to_set", handler_builder->build());
    }
    {
      auto *handler_builder = AddRouteConfigHandler::new_builder();
      std::ranges::for_each(_http_server_stage_config_targets.begin(),
                            _http_server_stage_config_targets.end(),
                            [handler_builder](auto &pair) {
                              handler_builder->add_http_server_config_sink(*pair.first, *pair.second);
                            });
      _handlers.emplace("add_route", handler_builder->build());
    }
    {
      auto *handler_builder = AddHttpValidatorSpecConfigHandler::new_builder();
      std::ranges::for_each(_validation_spec_stores.begin(),
                            _validation_spec_stores.end(),
                            [handler_builder](auto &pair) {
                              handler_builder->add_validation_store(*pair.first, *pair.second);
                            });
      _handlers.emplace("add_http_validation_spec", handler_builder->build());
    }
    {
      auto *handler_builder = AddHttpHeaderValidatorSpecConfigHandler::new_builder();
      std::ranges::for_each(_validation_spec_stores.begin(),
                            _validation_spec_stores.end(),
                            [handler_builder](auto &pair) {
                              handler_builder->add_validation_store(*pair.first, *pair.second);
                            });
      _handlers.emplace("add_http_header_validation_spec", handler_builder->build());
    }
    {
      auto *handler_builder = AddHttpPathValidatorSpecConfigHandler::new_builder();
      std::ranges::for_each(_validation_spec_stores.begin(),
                            _validation_spec_stores.end(),
                            [handler_builder](auto &pair) {
                              handler_builder->add_validation_store(*pair.first, *pair.second);
                            });
      _handlers.emplace("add_http_path_validation_spec", handler_builder->build());
    }
    {
      auto *handler_builder = AddHttpQueryValidatorSpecConfigHandler::new_builder();
      std::ranges::for_each(_validation_spec_stores.begin(),
                            _validation_spec_stores.end(),
                            [handler_builder](auto &pair) {
                              handler_builder->add_validation_store(*pair.first, *pair.second);
                            });
      _handlers.emplace("add_http_query_validation_spec", handler_builder->build());
    }
    {
      auto *handler_builder = AddThrottlerConfigHandler::new_builder();
      std::ranges::for_each(_throttler_stage_config_targets.begin(),
                            _throttler_stage_config_targets.end(),
                            [handler_builder](auto &pair) {
                              handler_builder->add_throttler_config_sink(*pair.first, *pair.second);
                            });
      _handlers.emplace("add_throttler", handler_builder->build());
    }
    {
      auto *handler_builder = UpdateThrottlerConfigHandler::new_builder();
      std::ranges::for_each(_throttler_stage_config_targets.begin(),
                            _throttler_stage_config_targets.end(),
                            [handler_builder](auto &pair) {
                              handler_builder->add_throttler_config_sink(*pair.first, *pair.second);
                            });
      _handlers.emplace("update_throttler", handler_builder->build());
    }
  }

  std::string execute_config_message(core::event_processing::IMessage *message) noexcept {

    auto *config_message = static_cast<ConfigurationMessage *>(message);
    std::stringstream ss;

    auto ite = _handlers.find(config_message->command());
    if (ite != _handlers.end()) {
      try {
        const auto &result = ite->second->handle(*config_message);
        ss << "[" << result._error_code << "][" << result._result << "][" << result._error_message << "]";
        return ss.str();
      } catch (const ConfigCommandParseException &ex) {
        ss << "[" << Errors::PARSE_ERROR << "][][" << ex.what() << "]";
        return ss.str();
      } catch (const std::runtime_error &ex) {
        int error_code = Errors::from_exception(ex);
        ss << "[" << error_code << "][][" << ex.what() << "]";
        return ss.str();
      } catch (const std::exception &ex) {
        ss << "[" << Errors::INTERNAL_ERROR << "][][" << ex.what() << "]";
        return ss.str();
      } catch (...) {
        ss << "[" << Errors::UNKNOWN_ERROR << "][][An unknown error occurred]";
        return ss.str();
      }
    } else {
      ss << "[" << Errors::NO_HANDLER << "][][No handler found to handle the command " << config_message->command()
         << "]";
      return ss.str();
    }
  }

  // builder
  class ConfiguratorServiceBuilder {
   public:
    ConfiguratorServiceBuilder *add_http_server_stage_config_sink(core::event_processing::ISink *sink,
                                                                  core::event_processing::ISink *posting_sink) {
      _service->_http_server_stage_config_targets.emplace_back(sink, posting_sink);
      return this;
    }

    ConfiguratorServiceBuilder *add_key_value_store(core::services::store_services::KeyValueStore *store,
                                                    core::event_processing::ISink *posting_sink) {
      _service->_key_value_stores.emplace_back(store, posting_sink);
      return this;
    }

    // TODO: remove this dependence on something inside stages. Services should not depend on stages ideally
    ConfiguratorServiceBuilder *add_validation_spec_store(core::services::store_services::KeyComplexValueStore<core::stages::http::validation::RequestValidator> *validation_spec_store,
                                                          core::event_processing::ISink *posting_sink) {
      _service->_validation_spec_stores.emplace_back(validation_spec_store, posting_sink);
      return this;
    }

    ConfiguratorServiceBuilder *add_throttler_stage_config_target(core::event_processing::ISink *throttler_stage_config_sink,
                                                                  core::event_processing::ISink *posting_sink) {
      _service->_throttler_stage_config_targets.emplace_back(throttler_stage_config_sink, posting_sink);
      return this;
    }

    ConfiguratorService *build() {
      return _service; // ownership is transferred
    }

   private:
    ConfiguratorService *_service = {new ConfiguratorService()};
  };

  static ConfiguratorServiceBuilder *newBuilder() {
    return new ConfiguratorServiceBuilder();
  }

  // these sinks should post the message through correct thread based on threading configuration
  std::list<std::pair<core::event_processing::ISink *, core::event_processing::ISink *>>
      _http_server_stage_config_targets;
  std::list<std::pair<core::services::store_services::KeyValueStore *, core::event_processing::ISink *>>
      _key_value_stores;
  std::list<std::pair<core::services::store_services::KeyComplexValueStore<core::stages::http::validation::RequestValidator> *,
                      core::event_processing::ISink *>> _validation_spec_stores;
  std::list<std::pair<core::event_processing::ISink *, core::event_processing::ISink *>>
      _throttler_stage_config_targets;

  std::unordered_map<std::string, IConfigurationCommandHandler *> _handlers;

};

}


