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

namespace core = adl::axp::core;

namespace adl::axp::gateway::services::configurator {

// TODO: rename this to TCPConfigurationMessage
class ConfigurationMessage : public core::stages::TcpRequestMessage {
 public:
  size_t message_type() const noexcept override {
    return core::extensions::compile_time_hash("adl::axp::gateway::stages::ConfigurationMessage");
  }

  const std::string &command() const { return _command; }
  std::string command() { return _command; }
  const std::list<std::string> &tokens() const {
    return _tokens;
  }

  class ConfigurationMessageBuilder {
   public:
    ConfigurationMessageBuilder &set_command(std::string_view command) {
      _message->_command = command;
      return *this;
    }

    ConfigurationMessageBuilder &add_token(std::string_view token) {
      _message->_tokens.emplace_back(token);
      return *this;
    }

    ConfigurationMessage *build() const {
      return _message;
    }

    ConfigurationMessageBuilder &reset() {
      _message = new ConfigurationMessage();
      return *this;
    }

   private:
    ConfigurationMessage *_message = {new ConfigurationMessage()};
  };

  static ConfigurationMessageBuilder &newBuilder() {
    return *(new ConfigurationMessageBuilder());
  }

 private:
  friend class ConfigurationMessageBuilder;

  std::string _command;
  std::list<std::string> _tokens;
};

}