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
 * @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-11-03.
 */

#pragma once

#include <core.h>

namespace adl::axp::gateway::services::configurator {

namespace core = adl::axp::core;
namespace event_processing = adl::axp::core::event_processing;
namespace stages = adl::axp::core::stages;
namespace services = adl::axp::core::services;

struct Errors {
  static constexpr int NO_ERROR = 0;
  static constexpr int PARSE_ERROR = -1;
  static constexpr int NO_HANDLER = -2;
  static constexpr int TIMEOUT = -3;
  static constexpr int INTERNAL_ERROR = -4;
  static constexpr int UNKNOWN_ERROR = -5;

  static int from_exception (const std::runtime_error& ex) {
    static constexpr int START_OFFSET = 1000;

    if(dynamic_cast<const services::store_services::KeyDoesNotExistException*>(&ex) != nullptr) {
      return -(START_OFFSET + 1);
    } else if(dynamic_cast<const services::store_services::InvalidListElementTypeException*>(&ex) != nullptr) {
      return -(START_OFFSET + 2);
    } else if(dynamic_cast<const services::store_services::InvalidSetElementTypeException*>(&ex) != nullptr) {
      return -(START_OFFSET + 3);
    } else if(dynamic_cast<const services::store_services::TypeMismatchException*>(&ex) != nullptr) {
      return -(START_OFFSET + 4);
    } else if(dynamic_cast<const stages::http::validation::HeaderValidatorException*>(&ex) != nullptr) {
      return -(START_OFFSET + 5);
    } else if(dynamic_cast<const stages::http::validation::QueryValidatorException*>(&ex) != nullptr) {
      return -(START_OFFSET + 6);
    } else if(dynamic_cast<const stages::http::validation::PathValidatorException*>(&ex) != nullptr) {
      return -(START_OFFSET + 7);
    } else if(dynamic_cast<const stages::http::validation::RequestValidatorException*>(&ex) != nullptr) {
      return -(START_OFFSET + 8);
    } else if(dynamic_cast<const stages::throttling::ThrottlerIdExistException*>(&ex) != nullptr) {
      return -(START_OFFSET + 9);
    } else if(dynamic_cast<const stages::throttling::ThrottlerDoesNotExistException*>(&ex) != nullptr) {
      return -(START_OFFSET + 10);
    } else if(dynamic_cast<const stages::throttling::InvalidThrottleTypeException*>(&ex) != nullptr) {
      return -(START_OFFSET + 11);
    } else if(dynamic_cast<const services::store_services::ValueExistInSetException*>(&ex) != nullptr) {
      return -(START_OFFSET + 12);
    } else {
      return START_OFFSET;
    }
  }
};

}