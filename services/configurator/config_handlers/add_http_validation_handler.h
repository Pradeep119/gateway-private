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
 * @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-09-17.
 */

#pragma once

#include <core.h>
#include "cxxopts_configuration_command_handler_base.h"
#include "../errors.h"
#include "common.h"

namespace core = adl::axp::core;

namespace adl::axp::gateway::services::configurator {

class AddHttpValidatorSpecConfigHandler : public CXXOptsConfigurationCommandHandlerBase {
 public:
  ConfigCommandResult handle(int &size, char **&tokens) noexcept(false) override {
    cxxopts::Options options("add_http_validation_spec", "Add a http request validator");
    options.add_options()
        ("k,key", "Unique key that must be used to store the validator", cxxopts::value<std::string>());

    try {
      const auto result = options.parse(size, tokens);
      const auto &key = result["key"].as<std::string>();

      auto apply_function =
          [=](core::services::store_services::KeyComplexValueStore<core::stages::http::validation::RequestValidator> *const store) -> void {
            store->add_value(key, new core::stages::http::validation::RequestValidator());
          };

      return apply<std::function<void(core::services::store_services::KeyComplexValueStore<core::stages::http::validation::RequestValidator> *const)>,
                   targets_t::iterator>(apply_function, _spec_stores.begin(), _spec_stores.end());

    } catch (const cxxopts::OptionSpecException &ex) {
      throw ConfigCommandParseException("add_throttler", ex.what());
    } catch (const cxxopts::OptionParseException &ex) {
      throw ConfigCommandParseException("add_throttler", ex.what());
    }
  }

  std::string_view get_usage_string() const noexcept override {
    return "";
  }

  class AddHttpValidatorSpecConfigHandlerBuilder {
   public:

    AddHttpValidatorSpecConfigHandlerBuilder &add_validation_store(core::services::store_services::KeyComplexValueStore<
        core::stages::http::validation::RequestValidator> &validator_store,
                                                                   core::event_processing::ISink &sink) {
      _handler->_spec_stores.emplace_back(&validator_store, &sink);
      return *this;
    }
    AddHttpValidatorSpecConfigHandler *build() {
      return _handler;
    }

   private:
    AddHttpValidatorSpecConfigHandler *_handler = {new AddHttpValidatorSpecConfigHandler()};
  };

  static AddHttpValidatorSpecConfigHandlerBuilder *new_builder() {
    return new AddHttpValidatorSpecConfigHandlerBuilder();
  }

 private:
  using targets_t = std::list<std::pair<core::services::store_services::KeyComplexValueStore<core::stages::http::validation::RequestValidator> *,
                                        core::event_processing::ISink *>>;
  targets_t _spec_stores;

  friend class AddHttpValidatorSpecConfigHandlerBuilder;
};

class AddHttpHeaderValidatorSpecConfigHandler : public CXXOptsConfigurationCommandHandlerBase {
 public:
  ConfigCommandResult handle(int &size, char **&tokens) noexcept(false) override {
    cxxopts::Options options("add_http_header_validation_spec", "Add a http header validation to an existing spec");
    options.add_options()
        ("k,key",
         "Key that must be used to find the request validation spec this header validation applies to",
         cxxopts::value<std::string>())
        ("h,header_key", "Header key this validation spec applies to", cxxopts::value<std::string>())
        ("t,validation_type", "Type of the validation [compulsory|data_type]", cxxopts::value<std::string>())
        ("d,data_type",
         "When validation_type is data_type, which data type [integer, decimal] ",
         cxxopts::value<std::string>());

    try {
      auto result = options.parse(size, tokens);

      auto &validation_type = result["validation_type"].as<std::string>();
      const auto &header_key = result["header_key"].as<std::string>();
      const auto &key = result["key"].as<std::string>();

      if (validation_type == "compulsory") {
        auto apply_function =
            [=](core::services::store_services::KeyComplexValueStore<core::stages::http::validation::RequestValidator> *store) -> void {
              auto *request_spec = store->get_value(key);
              request_spec->_header_validator.add_compulsory_validator(header_key);
            };

        return apply<std::function<void(core::services::store_services::KeyComplexValueStore<core::stages::http::validation::RequestValidator> *)>,
                     targets_t::iterator>(apply_function, _spec_stores.begin(), _spec_stores.end());

      } else if (validation_type == "data_type") {

        const auto &data_type = result["data_type"].as<std::string>();

        if (data_type == "integer") {

          auto apply_function =
              [=](core::services::store_services::KeyComplexValueStore<core::stages::http::validation::RequestValidator> *store) -> void {
                auto *request_spec = store->get_value(key);
                request_spec->_header_validator.add_data_type_validator<int64_t>(header_key);
              };

          return apply<std::function<void(core::services::store_services::KeyComplexValueStore<core::stages::http::validation::RequestValidator> *)>,
                       targets_t::iterator>(apply_function, _spec_stores.begin(), _spec_stores.end());

        } else if (data_type == "decimal") {
          auto apply_function =
              [=](core::services::store_services::KeyComplexValueStore<core::stages::http::validation::RequestValidator> *store) -> void {
                auto *request_spec = store->get_value(key);
                request_spec->_header_validator.add_data_type_validator<double>(header_key);
              };

          return apply<std::function<void(core::services::store_services::KeyComplexValueStore<core::stages::http::validation::RequestValidator> *)>,
                       targets_t::iterator>(apply_function, _spec_stores.begin(), _spec_stores.end());
        } else {
          throw ConfigCommandParseException("add_http_header_validation_spec",
                                            std::string("unknown data_type specified - ") + data_type);
        }
      } else {
        throw ConfigCommandParseException("add_http_header_validation_spec",
                                          std::string("unknown validation type specified - ") + validation_type);
      }

    } catch (const std::exception &ex) {
      throw ConfigCommandParseException("add_http_header_validation_spec", ex.what());
    }
  }

  std::string_view get_usage_string() const noexcept override {
    return "";
  }

  class AddHttpHeaderValidatorSpecConfigHandlerBuilder {
   public:

    AddHttpHeaderValidatorSpecConfigHandlerBuilder &add_validation_store(core::services::store_services::KeyComplexValueStore<
        core::stages::http::validation::RequestValidator> &validator_store,
                                                                         core::event_processing::ISink &sink) {
      _handler->_spec_stores.emplace_back(&validator_store, &sink);
      return *this;
    }
    AddHttpHeaderValidatorSpecConfigHandler *build() {
      return _handler;
    }

   private:
    AddHttpHeaderValidatorSpecConfigHandler *_handler = {new AddHttpHeaderValidatorSpecConfigHandler()};
  };

  static AddHttpHeaderValidatorSpecConfigHandlerBuilder *new_builder() {
    return new AddHttpHeaderValidatorSpecConfigHandlerBuilder();
  }

 private:
  //TODO: define this type somewhere (core::services::store_services::KeyComplexValueStore<core::stages::http::validation::RequestValidator>)
  using targets_t = std::list<std::pair<core::services::store_services::KeyComplexValueStore<core::stages::http::validation::RequestValidator> *,
                                        core::event_processing::ISink *>>;
  targets_t _spec_stores;

  friend class AddHttpHeaderValidatorSpecConfigHandlerBuilder;
};

class AddHttpPathValidatorSpecConfigHandler : public CXXOptsConfigurationCommandHandlerBase {
 public:
  ConfigCommandResult handle(int &size, char **&tokens) noexcept(false) override {
    cxxopts::Options options("add_http_path_validation_spec", "Add a http Path validation to an existing spec");
    options.add_options()
        ("k,key",
         "Key that must be used to find the request validation spec this path validation applies to",
         cxxopts::value<std::string>())
        ("p,path_key", "Path index this validation spec applies to", cxxopts::value<std::string>())
        ("t,validation_type", "Type of the validation [compulsory|data_type]", cxxopts::value<std::string>())
        ("d,data_type",
         "When validation_type is data_type, which data type [integer, decimal, string, boolean] ",
         cxxopts::value<std::string>());

    try {
      auto result = options.parse(size, tokens);

      auto &validation_type = result["validation_type"].as<std::string>();
      const auto &path_key = result["path_key"].as<std::string>();
      const auto &key = result["key"].as<std::string>();

      if (validation_type == "compulsory") {
        auto apply_function =
            [=](core::services::store_services::KeyComplexValueStore<core::stages::http::validation::RequestValidator> *store) -> void {
              auto *request_spec = store->get_value(key);
              request_spec->_path_validator.add_compulsory_validator(path_key);
            };

        return apply<std::function<void(core::services::store_services::KeyComplexValueStore<core::stages::http::validation::RequestValidator> *)>,
                     targets_t::iterator>(apply_function, _spec_stores.begin(), _spec_stores.end());

      } else if (validation_type == "data_type") {

        const auto &data_type = result["data_type"].as<std::string>();

        if (data_type == "integer") {

          auto apply_function =
              [=](core::services::store_services::KeyComplexValueStore<core::stages::http::validation::RequestValidator> *store) -> void {
                auto *request_spec = store->get_value(key);
                request_spec->_path_validator.add_data_type_validator<int64_t>(path_key);
              };

          return apply<std::function<void(core::services::store_services::KeyComplexValueStore<core::stages::http::validation::RequestValidator> *)>,
                       targets_t::iterator>(apply_function, _spec_stores.begin(), _spec_stores.end());

        } else if (data_type == "decimal") {
          auto apply_function =
              [=](core::services::store_services::KeyComplexValueStore<core::stages::http::validation::RequestValidator> *store) -> void {
                auto *request_spec = store->get_value(key);
                request_spec->_path_validator.add_data_type_validator<double>(path_key);
              };

          return apply<std::function<void(core::services::store_services::KeyComplexValueStore<core::stages::http::validation::RequestValidator> *)>,
                       targets_t::iterator>(apply_function, _spec_stores.begin(), _spec_stores.end());
        } else {
          throw ConfigCommandParseException("add_http_path_validation_spec",
                                            std::string("unknown data_type specified - ") + data_type);
        }
      } else {
        throw ConfigCommandParseException("add_http_path_validation_spec",
                                          std::string("unknown validation type specified - ") + validation_type);
      }

    } catch (const std::exception &ex) {
      throw ConfigCommandParseException("add_http_path_validation_spec", ex.what());
    }
  }

  std::string_view get_usage_string() const noexcept override {
    return "";
  }

  class AddHttpPathValidatorSpecConfigHandlerBuilder {
   public:

    AddHttpPathValidatorSpecConfigHandlerBuilder &add_validation_store(core::services::store_services::KeyComplexValueStore<
        core::stages::http::validation::RequestValidator> &validator_store,
                                                                       core::event_processing::ISink &sink) {
      _handler->_spec_stores.emplace_back(&validator_store, &sink);
      return *this;
    }
    AddHttpPathValidatorSpecConfigHandler *build() {
      return _handler;
    }

   private:
    AddHttpPathValidatorSpecConfigHandler *_handler = {new AddHttpPathValidatorSpecConfigHandler()};
  };

  static AddHttpPathValidatorSpecConfigHandlerBuilder *new_builder() {
    return new AddHttpPathValidatorSpecConfigHandlerBuilder();
  }

 private:
  //TODO: define this type somewhere (core::services::store_services::KeyComplexValueStore<core::stages::http::validation::RequestValidator>)
  using targets_t = std::list<std::pair<core::services::store_services::KeyComplexValueStore<core::stages::http::validation::RequestValidator> *,
                                        core::event_processing::ISink *>>;
  targets_t _spec_stores;

  friend class AddHttpPathValidatorSpecConfigHandlerBuilder;
};

class AddHttpQueryValidatorSpecConfigHandler : public CXXOptsConfigurationCommandHandlerBase {
 public:
  ConfigCommandResult handle(int &size, char **&tokens) noexcept(false) override {
    cxxopts::Options options("add_http_Query_validation_spec", "Add a http Query validation to an existing spec");
    options.add_options()
        ("k,key",
         "Key that must be used to find the request validation spec this query validation applies to",
         cxxopts::value<std::string>())
        ("q,query_key", "Query key this validation spec applies to", cxxopts::value<std::string>())
        ("t,validation_type", "Type of the validation [compulsory|data_type]", cxxopts::value<std::string>())
        ("d,data_type",
         "When validation_type is data_type, which data type [integer, decimal, string, boolean] ",
         cxxopts::value<std::string>());

    try {
      auto result = options.parse(size, tokens);

      auto &validation_type = result["validation_type"].as<std::string>();
      const auto &query_key = result["query_key"].as<std::string>();
      const auto &key = result["key"].as<std::string>();

      if (validation_type == "compulsory") {
        auto apply_function =
            [=](core::services::store_services::KeyComplexValueStore<core::stages::http::validation::RequestValidator> *store) -> void {
              auto *request_spec = store->get_value(key);
              request_spec->_query_validator.add_compulsory_validator(query_key);
            };

        return apply<std::function<void(core::services::store_services::KeyComplexValueStore<core::stages::http::validation::RequestValidator> *)>,
                     targets_t::iterator>(apply_function, _spec_stores.begin(), _spec_stores.end());

      } else if (validation_type == "data_type") {

        const auto &data_type = result["data_type"].as<std::string>();

        if (data_type == "integer") {

          auto apply_function =
              [=](core::services::store_services::KeyComplexValueStore<core::stages::http::validation::RequestValidator> *store) -> void {
                auto *request_spec = store->get_value(key);
                request_spec->_query_validator.add_data_type_validator<int64_t>(query_key);
              };

          return apply<std::function<void(core::services::store_services::KeyComplexValueStore<core::stages::http::validation::RequestValidator> *)>,
                       targets_t::iterator>(apply_function, _spec_stores.begin(), _spec_stores.end());

        } else if (data_type == "decimal") {
          auto apply_function =
              [=](core::services::store_services::KeyComplexValueStore<core::stages::http::validation::RequestValidator> *store) -> void {
                auto *request_spec = store->get_value(key);
                request_spec->_query_validator.add_data_type_validator<double>(query_key);
              };

          return apply<std::function<void(core::services::store_services::KeyComplexValueStore<core::stages::http::validation::RequestValidator> *)>,
                       targets_t::iterator>(apply_function, _spec_stores.begin(), _spec_stores.end());
        } else {
          throw ConfigCommandParseException("add_http_query_validation_spec",
                                            std::string("unknown data_type specified - ") + data_type);
        }
      } else {
        throw ConfigCommandParseException("add_http_query_validation_spec",
                                          std::string("unknown validation type specified - ") + validation_type);
      }

    } catch (const std::exception &ex) {
      throw ConfigCommandParseException("add_http_query_validation_spec", ex.what());
    }
  }

  std::string_view get_usage_string() const noexcept override {
    return "";
  }

  class AddHttpQueryValidatorSpecConfigHandlerBuilder {
   public:

    AddHttpQueryValidatorSpecConfigHandlerBuilder &add_validation_store(core::services::store_services::KeyComplexValueStore<
        core::stages::http::validation::RequestValidator> &validator_store,
                                                                        core::event_processing::ISink &sink) {
      _handler->_spec_stores.emplace_back(&validator_store, &sink);
      return *this;
    }
    AddHttpQueryValidatorSpecConfigHandler *build() {
      return _handler;
    }

   private:
    AddHttpQueryValidatorSpecConfigHandler *_handler = {new AddHttpQueryValidatorSpecConfigHandler()};
  };

  static AddHttpQueryValidatorSpecConfigHandlerBuilder *new_builder() {
    return new AddHttpQueryValidatorSpecConfigHandlerBuilder();
  }

 private:
  //TODO: define this type somewhere (core::services::store_services::KeyComplexValueStore<core::stages::http::validation::RequestValidator>)
  using targets_t = std::list<std::pair<core::services::store_services::KeyComplexValueStore<core::stages::http::validation::RequestValidator> *,
                                        core::event_processing::ISink *>>;
  targets_t _spec_stores;

  friend class AddHttpQueryValidatorSpecConfigHandlerBuilder;
};

}