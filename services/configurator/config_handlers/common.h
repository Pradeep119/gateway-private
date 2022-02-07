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
 * @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-11-04.
 */


#pragma once

#include <core.h>
#include "cxxopts_configuration_command_handler_base.h"
#include "../errors.h"

namespace core = adl::axp::core;

namespace adl::axp::gateway::services::configurator {

template<typename apply_function_t, typename return_t, typename target_t>
struct caller {
  void operator()(const apply_function_t &function, std::promise<return_t> &promise, target_t &target) {
    const auto &result = function(target);
    promise.set_value(result);
  }
};

template<typename apply_function_t, typename target_t>
struct caller<apply_function_t, void, target_t> {
  void operator()(const apply_function_t &function, std::promise<void> &promise, target_t &target) {
    function(target);
    promise.set_value();
  }
};

template<typename result_t>
ConfigCommandResult get_result(std::future<result_t> &future) {
  const auto &result = future.get();
  return ConfigCommandResult{Errors::NO_ERROR, std::to_string(result), ""};
}

template<>
inline
ConfigCommandResult get_result<void>(std::future<void> &future) {
  future.get();
  return ConfigCommandResult{Errors::NO_ERROR, "", ""};
}

template<typename apply_function_t, typename target_iterator_t>
ConfigCommandResult apply(const apply_function_t &function, target_iterator_t begin, target_iterator_t end) {

  using return_t = typename apply_function_t::result_type;

  auto *futures = new std::list<std::future<return_t>>{}; //TODO: avoid this

  std::ranges::for_each(begin, end, [futures, &function](auto &pair) {
    auto &target = pair.first;
    auto &target_posting_sink = pair.second;

    auto *promise = new std::promise<return_t>{}; //TODO: can we avoid this?
    futures->push_back(promise->get_future());

    core::event_processing::IMessage *exec_message =
        static_cast<core::event_processing::IMessage *>(malloc(sizeof(core::event_processing::messages::TaskExecutionMessage)));

    auto function_wrapper = [=]() mutable -> void {
      try {
        caller<apply_function_t, return_t, decltype(target)> caller;
        caller(function, *promise, target);
        if (nullptr != exec_message) delete exec_message;
      } catch (const std::exception &ex) {
        promise->set_exception(std::current_exception());
      }
    };

    target_posting_sink->on_message(new(exec_message)  core::event_processing::messages::TaskExecutionMessage(
        function_wrapper));
  });

  // wait for all futures
  for (auto &future: *futures) {
    const auto &status = future.wait_for(std::chrono::seconds(100)); //TODO: get the time out injected
    switch (status) {
      case std::future_status::ready:
      case std::future_status::deferred:
        // result is ready
        return get_result<return_t>(future); // throws if any exception occurred
        break;
      case std::future_status::timeout:
        return ConfigCommandResult{
            Errors::TIMEOUT, "", "Configuration request timed out"
        };

      default:
        return ConfigCommandResult{
            Errors::INTERNAL_ERROR, "", "Internal error occurred"
        };

    }
  }

  // if we are here, all went ok
  return ConfigCommandResult{
      Errors::NO_ERROR, "", ""
  };
}

}