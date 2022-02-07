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

#include <cxxopts.hpp>

#include "../configuration_message.h"
#include "../configuration_command_handler.h"

namespace core = adl::axp::core;

namespace adl::axp::gateway::services::configurator {

// this class can be used as the base for all handlers that use cxxopts to parse
// tokens. https://github.com/jarro2783/cxxopts
class CXXOptsConfigurationCommandHandlerBase : public IConfigurationCommandHandler {
 public:
  virtual ~CXXOptsConfigurationCommandHandlerBase() = default;

  ConfigCommandResult handle(const ConfigurationMessage &configuration_message) noexcept(false) override {
    std::list<std::string> tokens_copy = configuration_message.tokens();
    std::vector<char *> c_strings;
    for (auto &token: tokens_copy) {
      c_strings.push_back(&token.front());
    }

    int argc = c_strings.size();
    char **data = c_strings.data();
    return handle(argc, data);
  }

 protected:
  // derived classes can implement this
  virtual ConfigCommandResult handle(int &size, char **&tokens) noexcept(false) = 0;

};

}