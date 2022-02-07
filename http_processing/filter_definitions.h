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
 * @author Manusha Wijekoon (manusha.wijekoon@axiatadigitallabs.com) on 2021-11-12.
 */

#include <core.h>

namespace core = adl::axp::core;
namespace net = boost::asio;
namespace gateway = adl::axp::gateway;

namespace adl::axp::gateway::http {

struct WhiteBlackListFilterDefs {
  // whitelisting enabled at api, path and method level
  core::stages::filter_stage::WhitelistingStage::field_combination_t _wl_field_combinations{
      {
          {"ip_whitelist", "api"},
          {"http_info", "sender_ip_6"}
      },
      {
          {"ip_whitelist", "path"},
          {"http_info", "sender_ip_6"}
      },
      {
          {"ip_whitelist", "method"},
          {"http_info", "sender_ip_6"}
      },
      {
          {"ua_whitelist", "api"},
          {"http_info", "sender_user_agent"}
      },
      {
          {"ua_whitelist", "path"},
          {"http_info", "sender_user_agent"}
      },
      {
          {"ua_whitelist", "method"},
          {"http_info", "sender_user_agent"}
      }
  };

  // blacklisting enabled at api, path and method level
  core::stages::filter_stage::WhitelistingStage::field_combination_t _bl_field_combinations{
      {
          {"ip_blacklist", "api"},
          {"http_info", "sender_ip_6"}
      },
      {
          {"ip_blacklist", "path"},
          {"http_info", "sender_ip_6"}
      },
      {
          {"ip_blacklist", "method"},
          {"http_info", "sender_ip_6"}
      },
      {
          {"ua_blacklist", "api"},
          {"http_info", "sender_user_agent"}
      },
      {
          {"ua_blacklist", "path"},
          {"http_info", "sender_user_agent"}
      },
      {
          {"ua_blacklist", "method"},
          {"http_info", "sender_user_agent"}
      }
  };


//  // user agent whitelisting enabled at api, path and method level
//  core::stages::filter_stage::WhitelistingStage::field_combination_t _ua_wl_field_combinations{
//      {
//          {"ua_whitelist", "api"},
//          {"sender_details", "sender_user_agent"}
//      },
//      {
//          {"ua_whitelist", "path"},
//          {"sender_details", "sender_user_agent"}
//      },
//      {
//          {"ua_whitelist", "method"},
//          {"sender_details", "sender_user_agent"}
//      }
//  };
//  // user agent blacklisting enabled at api, path and method level
//  core::stages::filter_stage::WhitelistingStage::field_combination_t _ua_bl_field_combinations{
//      {
//          {"ua_blacklist", "api"},
//          {"sender_details", "sender_user_agent"}
//      },
//      {
//          {"ua_blacklist", "path"},
//          {"sender_details", "sender_user_agent"}
//      },
//      {
//          {"ua_blacklist", "method"},
//          {"sender_details", "sender_user_agent"}
//      }
//  };
};
}