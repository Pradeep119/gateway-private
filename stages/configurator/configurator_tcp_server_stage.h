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
 * @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-09-15.
 */

#include <list>
#include <core.h>
#include "../../services/configurator/configuration_message.h"

namespace adl::axp::gateway::stages::configurator {
// thread safe - no
struct ConfigMessageAssembler {

  std::list<adl::axp::core::stages::TcpRequestMessage*> assemble(char *data, std::size_t size, void *tag) {
    //TODO: optimize this list
    std::list<adl::axp::core::stages::TcpRequestMessage*> messages;
    size_t used_data = 0;
    while(used_data < size) {
      auto [used_count, message] = parse_message(data + used_data, size - used_data);
      used_data += used_count;
      if(nullptr != message) messages.push_back(message);
    }

    return messages;
  }

 private:
  std::pair<size_t, adl::axp::core::stages::TcpRequestMessage*> parse_message(char *data, std::size_t size) {
    for (std::size_t i = 0; i < size; ++i) {
      if (data[i] == '\r') {
        // end of command
        on_token(std::string_view(_buffer, _write_position));
        auto *message = _message_builder.build();

        // prepare for the next one
        _write_position = 0;
        _message_builder.reset();
        _command_received = false;

        return {i + 1, message}; // +1 for \r which is now used and following \n

      } else if (data[i] == '\"') {
        // start or end of a string
        _string_in_progress = !_string_in_progress;
      } else if (!_string_in_progress && data[i] == ' ') {
        // new token
        on_token(std::string_view(_buffer, _write_position));
        _write_position = 0;
      } else {
        _buffer[_write_position++] = data[i];
      }
    }
    return {size, nullptr};
  }


 private:
  void on_token(std::string_view token) {
//    std::cout << "on token " << std::string(token) << std::endl;
    _message_builder.add_token(token); // command is also a token
    if (!_command_received) {
      _message_builder.set_command(token);
      _command_received = true;
    }
  }

  bool _command_received;
  adl::axp::gateway::services::configurator::ConfigurationMessage::ConfigurationMessageBuilder _message_builder;
  char _buffer[5000] = {0};
  std::size_t _write_position = 0;
  bool _string_in_progress = {false};
};

using ConfiguratorTcpServerStage = core::stages::TcpServerStage<ConfigMessageAssembler>;

}
