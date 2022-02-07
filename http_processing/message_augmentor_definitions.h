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

struct MessageAugmentorDefs {
  // message augmentors

  // expands various information about api call and put them in diffferent fields
  // following uses the value of the field <http_info, route_key_2>
  // extracts operationid, apiid, resourcepath and method which are encoded inside [ and ]
  core::stages::message_augmentor::TokenExtractingAugmentorDefs::Config _cfg_token{
      {"http_info", "route_key_2"},
      '[',
      ']',
      4,
      {{"api_info", "api_id"}, {"api_info", "path_id"}, {"api_info", "method"}, {"api_info", "resource"}}
  };

  core::stages::message_augmentor::MessageAugmentor<core::stages::message_augmentor::TokenExtractingAugmentorDefs::Config>
      _aug_extract_api_info{"message augmentor 1", "expands parts of api call", _cfg_token,
                            core::stages::message_augmentor::TokenExtractingAugmentorDefs::augment_operator};

  struct WhiteListing {

    // Following uses various api request information to populate white and black list
    core::stages::message_augmentor::ConcatenatingAugmentorDefs::Config _cfg_api_ip{
        {
            {"api_info", "api_id"},
        },
        "/",
        "whitelists",
        "sender_ip_address",
        "ip_whitelist",
        "api"
    };
    core::stages::message_augmentor::MessageAugmentor<core::stages::message_augmentor::ConcatenatingAugmentorDefs::Config>
        _aug_api_ip{"wl_api", "creates the lookup field for api id based white listing", _cfg_api_ip,
                    core::stages::message_augmentor::ConcatenatingAugmentorDefs::augment_operator};

    core::stages::message_augmentor::ConcatenatingAugmentorDefs::Config _cfg_path_ip{
        {
            {"api_info", "api_id"},
            {"api_info", "path_id"},
        },
        "/",
        "whitelists",
        "sender_ip_address",
        "ip_whitelist",
        "path"
    };
    core::stages::message_augmentor::MessageAugmentor<core::stages::message_augmentor::ConcatenatingAugmentorDefs::Config>
        _aug_path_ip{"wl_path", "creates the lookup field for path id based white listing", _cfg_path_ip,
                     core::stages::message_augmentor::ConcatenatingAugmentorDefs::augment_operator};

    core::stages::message_augmentor::ConcatenatingAugmentorDefs::Config _cfg_method_ip{
        {
            {"api_info", "api_id"},
            {"api_info", "path_id"},
            {"api_info", "method"},
        },
        "/",
        "whitelists",
        "sender_ip_address",
        "ip_whitelist",
        "method"
    };
    core::stages::message_augmentor::MessageAugmentor<core::stages::message_augmentor::ConcatenatingAugmentorDefs::Config>
        _aug_method_ip{"wl_method", "creates the lookup field for method id based white listing", _cfg_method_ip,
                       core::stages::message_augmentor::ConcatenatingAugmentorDefs::augment_operator};

    //
    core::stages::message_augmentor::ConcatenatingAugmentorDefs::Config _cfg_api_ua{
        {
            {"api_info", "api_id"},
        },
        "/",
        "whitelists",
        "sender_user_agent",
        "ua_whitelist",
        "api"
    };
    core::stages::message_augmentor::MessageAugmentor<core::stages::message_augmentor::ConcatenatingAugmentorDefs::Config>
        _aug_api_ua{"wl_api", "creates the lookup field for api id based white listing", _cfg_api_ua,
                    core::stages::message_augmentor::ConcatenatingAugmentorDefs::augment_operator};

    core::stages::message_augmentor::ConcatenatingAugmentorDefs::Config _cfg_path_ua{
        {
            {"api_info", "api_id"},
            {"api_info", "path_id"},
        },
        "/",
        "whitelists",
        "sender_user_agent",
        "ua_whitelist",
        "path"
    };
    core::stages::message_augmentor::MessageAugmentor<core::stages::message_augmentor::ConcatenatingAugmentorDefs::Config>
        _aug_path_ua{"wl_path", "creates the lookup field for path id based white listing", _cfg_path_ua,
                     core::stages::message_augmentor::ConcatenatingAugmentorDefs::augment_operator};

    core::stages::message_augmentor::ConcatenatingAugmentorDefs::Config _cfg_method_ua{
        {
            {"api_info", "api_id"},
            {"api_info", "path_id"},
            {"api_info", "method"},
        },
        "/",
        "whitelists",
        "sender_user_agent",
        "ua_whitelist",
        "method"
    };
    core::stages::message_augmentor::MessageAugmentor<core::stages::message_augmentor::ConcatenatingAugmentorDefs::Config>
        _aug_method_ua{"wl_method", "creates the lookup field for method id based white listing", _cfg_method_ua,
                       core::stages::message_augmentor::ConcatenatingAugmentorDefs::augment_operator};
  } _white_listing;

  // black lists
  struct BlackListing {
    core::stages::message_augmentor::ConcatenatingAugmentorDefs::Config _cfg_api_ip{
        {
            {"api_info", "api_id"},
        },
        "/",
        "blacklists",
        "sender_ip_address",
        "ip_blacklist",
        "api"
    };
    core::stages::message_augmentor::MessageAugmentor<core::stages::message_augmentor::ConcatenatingAugmentorDefs::Config>
        _aug_api_ip{"wl_api", "creates the lookup field for api id based white listing", _cfg_api_ip,
                    core::stages::message_augmentor::ConcatenatingAugmentorDefs::augment_operator};

    core::stages::message_augmentor::ConcatenatingAugmentorDefs::Config _cfg_path_ip{
        {
            {"api_info", "api_id"},
            {"api_info", "path_id"},
        },
        "/",
        "blacklists",
        "sender_ip_address",
        "ip_blacklist",
        "path"
    };
    core::stages::message_augmentor::MessageAugmentor<core::stages::message_augmentor::ConcatenatingAugmentorDefs::Config>
        _aug_path_ip{"wl_path", "creates the lookup field for path id based white listing", _cfg_path_ip,
                     core::stages::message_augmentor::ConcatenatingAugmentorDefs::augment_operator};

    core::stages::message_augmentor::ConcatenatingAugmentorDefs::Config _cfg_method_ip{
        {
            {"api_info", "api_id"},
            {"api_info", "path_id"},
            {"api_info", "method"},
        },
        "/",
        "blacklists",
        "sender_ip_address",
        "ip_blacklist",
        "method"
    };
    core::stages::message_augmentor::MessageAugmentor<core::stages::message_augmentor::ConcatenatingAugmentorDefs::Config>
        _aug_method_ip{"wl_method", "creates the lookup field for method id based white listing", _cfg_method_ip,
                       core::stages::message_augmentor::ConcatenatingAugmentorDefs::augment_operator};

    //
    core::stages::message_augmentor::ConcatenatingAugmentorDefs::Config _cfg_api_ua{
        {
            {"api_info", "api_id"},
        },
        "/",
        "blacklists",
        "sender_user_agent",
        "ua_blacklist",
        "api"
    };
    core::stages::message_augmentor::MessageAugmentor<core::stages::message_augmentor::ConcatenatingAugmentorDefs::Config>
        _aug_api_ua{"wl_api", "creates the lookup field for api id based white listing", _cfg_api_ua,
                    core::stages::message_augmentor::ConcatenatingAugmentorDefs::augment_operator};

    core::stages::message_augmentor::ConcatenatingAugmentorDefs::Config _cfg_path_ua{
        {
            {"api_info", "api_id"},
            {"api_info", "path_id"},
        },
        "/",
        "blacklists",
        "sender_user_agent",
        "ua_blacklist",
        "path"
    };
    core::stages::message_augmentor::MessageAugmentor<core::stages::message_augmentor::ConcatenatingAugmentorDefs::Config>
        _aug_path_ua{"wl_path", "creates the lookup field for path id based white listing", _cfg_path_ua,
                     core::stages::message_augmentor::ConcatenatingAugmentorDefs::augment_operator};

    core::stages::message_augmentor::ConcatenatingAugmentorDefs::Config _cfg_method_ua{
        {
            {"api_info", "api_id"},
            {"api_info", "path_id"},
            {"api_info", "method"},
        },
        "/",
        "blacklists",
        "sender_user_agent",
        "ua_blacklist",
        "method"

    };
    core::stages::message_augmentor::MessageAugmentor<core::stages::message_augmentor::ConcatenatingAugmentorDefs::Config>
        _aug_method_ua{"wl_method", "creates the lookup field for method id based white listing", _cfg_method_ua,
                       core::stages::message_augmentor::ConcatenatingAugmentorDefs::augment_operator};

  } _black_listing;

  struct LoadBalancing {
    core::stages::message_augmentor::ConcatenatingAugmentorDefs::Config _cfg_api{
        {
            {"api_info", "api_id"}
        },
        "/",
        "rest_api_be_pools",
        "",
        "backendpools",
        "api"
    };
    core::stages::message_augmentor::MessageAugmentor<core::stages::message_augmentor::ConcatenatingAugmentorDefs::Config>
        _aug_api{"wl_method", "creates the lookup field for method id based white listing", _cfg_api,
                 core::stages::message_augmentor::ConcatenatingAugmentorDefs::augment_operator};
  } _load_balancing;

  struct HttpValidation {
    core::stages::message_augmentor::ConcatenatingAugmentorDefs::Config _cfg_validations{
        {
            {"api_info", "api_id"},
            {"api_info", "path_id"},
            {"api_info", "method"},
        },
        "/",
        "validators",
        "",
        "http_info",
        "http_validator_spec_key"
    };
    core::stages::message_augmentor::MessageAugmentor<core::stages::message_augmentor::ConcatenatingAugmentorDefs::Config>
        _aug_api_val{"http_validations", "creates the lookup field for http validators", _cfg_validations,
                 core::stages::message_augmentor::ConcatenatingAugmentorDefs::augment_operator};
  } _http_validation;

  std::list<core::stages::message_augmentor::MessageAugmentorBase *>
      _augmentors{&_aug_extract_api_info,
                  &_white_listing._aug_api_ip, &_white_listing._aug_path_ip, &_white_listing._aug_method_ip,
                  &_white_listing._aug_api_ua, &_white_listing._aug_path_ua, &_white_listing._aug_method_ua,
                  &_black_listing._aug_api_ip, &_black_listing._aug_path_ip, &_black_listing._aug_method_ip,
                  &_black_listing._aug_api_ua, &_black_listing._aug_path_ua, &_black_listing._aug_method_ua,
                  &_load_balancing._aug_api,
                  &_http_validation._aug_api_val};

};

}

