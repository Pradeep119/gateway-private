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

namespace core = adl::axp::core;

namespace adl::axp::gateway::services::configurator {

// exceptions
class ConfigCommandParseException : public std::runtime_error {
 public:
  ConfigCommandParseException(std::string_view config_command, std::string_view error_msg) :
      std::runtime_error(
          "Error while parsing command - " + std::string(config_command) + "[" + std::string(error_msg) + "]") {
  }
};

// result
struct ConfigCommandResult {
  int _error_code; // 0 if no error
  std::string _result; // string representing the result of the command
  std::string _error_message; // only applies if error_code is not zero.
};

interface IConfigurationCommandHandler {
  virtual ~IConfigurationCommandHandler() = default;

  virtual ConfigCommandResult handle(const ConfigurationMessage &configuration_message) noexcept(false) = 0;
  virtual std::string_view get_usage_string() const noexcept = 0;
};
}