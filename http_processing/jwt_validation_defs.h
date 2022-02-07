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
 * @author Braveenan Sritharan (braveenan.sritharan@axiatadigitallabs.com) on 2021/12/23.
 * @author Janith Priyankara (janith.priyankara@axiatadigitallabs.com) on 2022-01-20.
 */


namespace adl::axp::gateway::JwtValidation {

struct JwtValidationDefs {
  const core::stages::jwt_token::JwtValidationStage::field_combination_t _token_field = {"http_info", "access_token"};
};

struct RSAKeys {
  const std::string _rsa_pub_key_1 = R"(-----BEGIN PUBLIC KEY-----
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAxeqoZYbQ/Sr8DOFQ+/qb
EbCp6Vzb5hzH7oa3hf2FZxRKF0H6b8COMzz8+0mvEdYVvb/31jMEL2CIQhkQRol1
IruD6nBOmkjuXJSBficklMaJZORhuCrB4roHxzoG19aWmscA0gnfBKo2oGXSjJmn
ZxIh+2X6syHCfyMZZ00LzDyrgoXWQXyFvCA2ax54s7sKiHOM3P4A9W4QUwmoEi4H
QmPgJjIM4eGVPh0GtIANN+BOQ1KkUI7OzteHCTLu3VjxM0sw8QRayZdhniPF+U9n
3fa1mO4KLBsW4mDLjg8R/JuAGTX/SEEGj0B5HWQAP6myxKFz2xwDaCGvT+rdvkkt
OwIDAQAB
-----END PUBLIC KEY-----)";
};
}

