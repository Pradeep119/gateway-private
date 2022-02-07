#
#
#      © Copyrights 2022 Axiata Digital Labs Pvt Ltd.
#      All Rights Reserved.
#
#      These material are unpublished, proprietary, confidential source
#      code of Axiata Digital Labs Pvt Ltd (ADL) and constitute a TRADE
#      SECRET of ADL.
#
#      ADL retains all title to and intellectual property rights in these
#      materials.
#
#
#      @author Janith Priyankara (janith.priyankara@axiatadigitallabs.com) on 2022/01/05.
#
#


class ConfigurationSDK:
    @staticmethod
    def add_api(api_id, api_name, api_description, protocol):
        str_buff = "add_to_set --key " + Constants.ALL_API_KEY + " -t string -v " + wrap_in_quotes(api_id)
        str_buff1 = "add_key_value --key " + "rest_apis/" + wrap_in_quotes(
            api_id + "/name") + " -t string -v " + wrap_in_quotes(
            api_name)
        str_buff2 = "add_key_value --key " + "rest_apis/" + wrap_in_quotes(
            api_id + "/desc") + " -t string -v " + wrap_in_quotes(
            api_description)
        str_buff3 = "add_key_value --key " + "rest_apis/" + wrap_in_quotes(
            api_id + "/protocol") + " -t string -v " + to_string(
            protocol)
        return [str_buff, str_buff1, str_buff2, str_buff3]

    @staticmethod
    def add_api_operation(api_id, path_id, http_method, resource_path):
        str_buff = "add_route --route_key_1 " + api_id + " --route_key_2 " + "[" + api_id + "]" + "[" + path_id + "]" + "[" + http_method + "]" + "[" + resource_path + "]" + " --resource " + resource_path + " --method " + http_method
        return [str_buff]

    @staticmethod
    def add_api_back_end_pool(pool_key, pool_type, protocol):
        str_buff = "add_to_set --key " + Constants.ALL_BE_POOL_KEY + " -t string -v " + pool_key
        str_buff1 = "add_key_value --key " + pool_key + "/type" + " -t string -v " + wrap_in_quotes(pool_type)
        str_buff2 = "add_key_value --key " + pool_key + "/protocol" + " -t string -v " + wrap_in_quotes(protocol)
        return [str_buff, str_buff1, str_buff2]

    @staticmethod
    def add_api_back_end_pool_api_scope(api_id, pool_type, protocol):
        pool_key = Constants.REST_API_BE_POOL_KEY_PREFIX + "/" + api_id
        return [pool_key, ConfigurationSDK.add_api_back_end_pool(pool_key, pool_type, protocol)]

    @staticmethod
    def add_api_back_end_pool_path_scope(api_id, path_id, pool_type, protocol):
        pool_key = Constants.REST_API_BE_POOL_KEY_PREFIX + "/" + api_id + "/" + path_id
        return [pool_key, ConfigurationSDK.add_api_back_end_pool(pool_key, pool_type, protocol)]

    @staticmethod
    def add_api_back_end_pool_method_scope(api_id, path_id, http_method, pool_type, protocol):
        pool_key = Constants.REST_API_BE_POOL_KEY_PREFIX + "/" + api_id + "/" + path_id + "/" + http_method
        return [pool_key, ConfigurationSDK.add_api_back_end_pool(pool_key, pool_type, protocol)]

    @staticmethod
    def add_back_end(backend_id, pool_id, host, port):
        back_end_key = Constants.REST_API_BE_KEY_PREFIX + "/" + backend_id
        str_buff = "add_to_list" + " --key " + pool_id + " --value_type string --value " + back_end_key
        str_buff1 = "add_key_value --key " + back_end_key + "/host" + " --value_type string" + " --value " + host
        str_buff2 = "add_key_value --key " + back_end_key + "/port" + " --value_type string" + " --value " + str(port)
        return [str_buff, str_buff1, str_buff2]

    @staticmethod
    def add_api_operation_validator(api_id, path_id, http_method):
        header_validation_key = Constants.HTTP_VALIDATION_PREFIX + "/" + api_id + "/" + path_id + "/" + http_method
        str_buff = "add_http_validation_spec" + " -k " + header_validation_key
        return [header_validation_key, [str_buff]]

    @staticmethod
    def add_compulsory_header_validator(validator_id, header_name):
        str_buff = "add_http_header_validation_spec" + " -k " + validator_id + " -h " + header_name + " -t " + "compulsory"
        return [str_buff]

    @staticmethod
    def add_data_type_header_validator(validator_id, header_name, data_type):
        str_buff = "add_http_header_validation_spec" + " -k " + validator_id + " -h " + header_name + " -t " + "data_type" + " -d " + data_type
        return [str_buff]

    @staticmethod
    def add_compulsory_path_validator(validator_id, path_id):
        str_buff = "add_http_path_validation_spec" + " -k " + validator_id + " -p " + path_id + " -t " + "compulsory"
        return [str_buff]

    @staticmethod
    def add_data_type_path_validator(validator_id, path_id, data_type):
        str_buff = "add_http_path_validation_spec" + " -k " + validator_id + " -p " + path_id + " -t " + "data_type" + " -d " + data_type
        return [str_buff]

    @staticmethod
    def add_compulsory_query_validator(validator_id, query_id):
        str_buff = "add_http_query_validation_spec" + " -k " + validator_id + " -q " + query_id + " -t " + "compulsory"
        return [str_buff]

    @staticmethod
    def add_data_type_query_validator(validator_id, query_id, data_type):
        str_buff = "add_http_query_validation_spec" + " -k " + validator_id + " -q " + query_id + " -t " + "data_type" + " -d " + data_type
        return [str_buff]

    @staticmethod
    def add_throttler_api_scope(api_id, throttler_type, window_size, allowed_count):
        str_buff = "add_throttler --key " + api_id + " --type " + throttler_type + " --queuing false" + " --window_duration " + str(
            window_size) + " --allowed_count " + str(allowed_count)
        return [str_buff]

    @staticmethod
    def add_throttler_path_scope(api_id, path_id, throttler_type, window_size, allowed_count):
        str_buff = "add_throttler --key " + api_id + "/" + path_id + " --type " + throttler_type + " --queuing false" + " --window_duration " + str(
            window_size) + " --allowed_count " + str(allowed_count)
        return [str_buff]

    @staticmethod
    def add_to_wb_list(wl_key, entry):
        str_buff = "add_to_set -k " + wl_key + " -t " + "string" + " -v " + entry
        return [str_buff]

    @staticmethod
    def add_to_black_list_api_scope(api_id, entry, property_wb):
        wl_key = Constants.BLACK_LIST_PREFIX + "/" + api_id + "/" + property_wb
        return ConfigurationSDK.add_to_wb_list(wl_key, entry)

    @staticmethod
    def add_to_black_list_path_scope(api_id, path_id, entry, property_wb):
        wl_key = Constants.BLACK_LIST_PREFIX + "/" + api_id + "/" + path_id + "/" + property_wb
        return ConfigurationSDK.add_to_wb_list(wl_key, entry)

    @staticmethod
    def add_to_black_list_method_scope(api_id, path_id, http_method, entry, property_wb):
        wl_key = Constants.BLACK_LIST_PREFIX + "/" + api_id + "/" + path_id + "/" + http_method + "/" + property_wb
        return ConfigurationSDK.add_to_wb_list(wl_key, entry)

    @staticmethod
    def add_to_white_list_api_scope(api_id, entry, property_wb):
        wl_key = Constants.WHITE_LIST_PREFIX + "/" + api_id + "/" + property_wb
        return ConfigurationSDK.add_to_wb_list(wl_key, entry)

    @staticmethod
    def add_to_white_list_path_scope(api_id, path_id, entry, property_wb):
        wl_key = Constants.WHITE_LIST_PREFIX + "/" + api_id + "/" + path_id + "/" + property_wb
        return ConfigurationSDK.add_to_wb_list(wl_key, entry)

    @staticmethod
    def add_to_white_list_method_scope(api_id, path_id, http_method, entry, property_wb):
        wl_key = Constants.WHITE_LIST_PREFIX + "/" + api_id + "/" + path_id + "/" + http_method + "/" + property_wb
        return ConfigurationSDK.add_to_wb_list(wl_key, entry)


class Constants:
    ALL_API_KEY = "all_rest_apis"
    ALL_BE_POOL_KEY = "all_backend_pools"
    REST_API_BE_POOL_KEY_PREFIX = "rest_api_be_pools"
    REST_API_BE_KEY_PREFIX = "rest_api_backends"
    WHITE_LIST_PREFIX = "whitelists"
    BLACK_LIST_PREFIX = "blacklists"
    HTTP_VALIDATION_PREFIX = "validators"


class BWListProperty:
    SENDER_IP_ADDRESS = "sender_ip_address"
    SENDER_USER_AGENT = "sender_user_agent"


class BEPoolType:
    SOAP = "SOAP"
    REST = "REST"


switcher_protocols = {
    "HTTP": "http",
    "HTTPS": 'https',
    "HTTP_HTTPS": 'http|https',
}


def to_string(protocol):
    return switcher_protocols.get(protocol)


def wrap_in_quotes(str_in):
    return "\"" + str_in + "\""
