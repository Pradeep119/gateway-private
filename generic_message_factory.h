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
 * @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-09-27.
 */

#pragma once

#include <core.h>

namespace core = adl::axp::core;

namespace adl::axp::gateway {

template<class message_t>
class MessageFactory {
 public:
  message_t *create() {
    return new message_t();
  }
};

}