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
* @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-09-24.
*/

#pragma once

#include <core.h>

/**
 * Introduces a delay from in to out.
 */
namespace adl::axp::gateway::stages {

/**
 * Unwraps dropped http requests and forwards the dropped message
 */
class UnWrapStage : public core::event_processing::details::BaseStage,
                    public core::event_processing::ISink {
 public:
  UnWrapStage() : core::event_processing::details::BaseStage("unwrap") {
    register_source("out", &_out);
    register_sink("in", this);
  }
  virtual void on_message(core::event_processing::IMessage *message) noexcept {
    adl::axp::core::stages::details::http_dispatcher::HttpDroppedRequestMessage *dropped_request =
        static_cast<adl::axp::core::stages::details::http_dispatcher::HttpDroppedRequestMessage *>(message);
    _out.get_sink()->on_message(&dropped_request->get_dropped_message());
    //    dropped_message_pool.free(dropped_request);
  }

  core::event_processing::details::BaseSource _out;
};

}