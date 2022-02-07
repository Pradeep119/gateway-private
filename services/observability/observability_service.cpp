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
 * @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-10-08.
 */

#include <opentelemetry/metrics/meter_provider.h>
#include <opentelemetry/metrics/provider.h>
#include <opentelemetry/sdk/metrics/meter_provider.h>

#include "observability_service.h"

namespace adl::axp::gateway::services::observability {

void ObservabilityService::start() {
  // Initialize and set the global MeterProvider
  auto provider = opentelemetry::nostd::shared_ptr<metrics_api::MeterProvider>(new sdkmetrics::MeterProvider);
  opentelemetry::metrics::Provider::SetMeterProvider(provider);

  opentelemetry::nostd::shared_ptr<metrics_api::Meter> meter = provider->GetMeter("Http Server Stage");

  // Create the controller with Stateless Metrics Processor
  auto* controller = new sdkmetrics::PushController (
      meter,
      std::unique_ptr<sdkmetrics::MetricsExporter>(
          new opentelemetry::exporter::metrics::OStreamMetricsExporter),
          std::shared_ptr<sdkmetrics::MetricsProcessor>(
              new opentelemetry::sdk::metrics::UngroupedMetricsProcessor(false)),
              .05);

  controller->start();
}

}



