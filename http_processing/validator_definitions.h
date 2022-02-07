/*
 * © Copyrights 2022 Axiata Digital Labs Pvt Ltd.
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
 * @author Janith Priyankara (janith.priyankara@axiatadigitallabs.com) on 2022-01-20.
 */

#include <core.h>

namespace core = adl::axp::core;
namespace net = boost::asio;
namespace gateway = adl::axp::gateway;

namespace adl::axp::gateway::http {

struct HttpHeaderValidatorDefs {
  core::stages::http::validation::HttpRequestValidationStage::field_combination_t
      _http_validator_key_field_combination{"http_info", "http_validator_spec_key"};
  std::string _header_field_group = "http_headers";
  std::string _query_field_group = "query_parameters";
  std::string _path_field_group = "path_parameters";
};

}
