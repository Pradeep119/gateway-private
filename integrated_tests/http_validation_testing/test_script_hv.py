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


import json
import socket

import pytest

from common.configuration_sdk import ConfigurationSDK
from common.gateway_process import GatewayProcess
from common.wiremock_process import WiremockProcess

# artifact Paths
Gateway_artifact_path = "~/development/axp/nextgen/gateway/cmake-build-debug/./gateway"
arg1 = 4000
arg2 = 2500


@pytest.fixture(scope="function")
def init_and_configure():
    api_8080_data = {"request": {"method": "GET", "url": "/test"},
                     "response": {"status": 200, "body": "Hello world",
                                  "headers": {"Content-Type": "application/json", "charset": "UTF-8"}}}
    api_8081_data = {"request": {"method": "GET", "url": "/test"},
                     "response": {"status": 200, "body": "Hello world",
                                  "headers": {"Content-Type": "application/json", "charset": "UTF-8"}}}
    end_test()
    WiremockProcess.start(8080)
    WiremockProcess.start(8081)
    WiremockProcess.configurator("8080", json.dumps(api_8080_data))
    WiremockProcess.configurator("8081", json.dumps(api_8081_data))
    GatewayProcess.start(Gateway_artifact_path, arg1, arg2)

    command_list = []
    command_list += ConfigurationSDK.add_api("api_1", "pet store", "sample per store api", "HTTP")
    command_list += ConfigurationSDK.add_api_operation("api_1", "path_1", "GET", "/test_1/:id1/sub/:id2")
    command_list += ConfigurationSDK.add_api_operation("api_1", "path_2", "GET", "/test_2")
    command_list += ConfigurationSDK.add_api("api_2", "toy store", "sample per store api", "HTTP")
    command_list += ConfigurationSDK.add_api_operation("api_2", "path_1", "GET", "/foor_1")
    [pool_key1, _add_backend_pool_list1] = ConfigurationSDK.add_api_back_end_pool_api_scope("api_1", "REST", "HTTP")
    command_list += _add_backend_pool_list1
    command_list += ConfigurationSDK.add_back_end("be_1", pool_key1, "localhost", 8080)
    command_list += ConfigurationSDK.add_back_end("be_2", pool_key1, "localhost", 8081)
    [pool_key2, _add_backend_pool_list2] = ConfigurationSDK.add_api_back_end_pool_api_scope("api_2", "REST", "HTTP")
    command_list += ConfigurationSDK.add_back_end("be_2", pool_key2, "localhost", 8081)
    GatewayProcess.configure_low_level(command_list)


def end_test():
    WiremockProcess.shutdown(8080)
    WiremockProcess.shutdown(8081)
    GatewayProcess.stop()


def test_http_validation_1(init_and_configure):
    command_list = []
    [validator_id1, _add_api_operation_validator1] = ConfigurationSDK.add_api_operation_validator("api_1", "path_2",
                                                                                                  "GET")
    command_list += _add_api_operation_validator1
    command_list += ConfigurationSDK.add_compulsory_header_validator(validator_id1, "h1")
    [validator_id2, _add_api_operation_validator2] = ConfigurationSDK.add_api_operation_validator("api_1", "path_1",
                                                                                                  "GET")
    command_list += _add_api_operation_validator2
    command_list += ConfigurationSDK.add_data_type_path_validator(validator_id2, "id1", "integer")
    command_list += ConfigurationSDK.add_compulsory_header_validator(validator_id2, "h1")
    command_list += ConfigurationSDK.add_compulsory_query_validator(validator_id2, "q1")
    GatewayProcess.configure_low_level(command_list)

    headers = {"h1": "This is a compulsory header"}
    decoded_response1 = GatewayProcess.http_get("localhost:4000", "/test_1/100/sub/14", headers, "q1=12")
    assert decoded_response1 == "Hello world"

    headers = {"h1": "This is a compulsory header", "h2": "This is not a compulsory header"}
    decoded_response1 = GatewayProcess.http_get("localhost:4000", "/test_1/100/sub/14", headers, "q1=12")
    assert decoded_response1 == "Hello world"

    headers = {"h2": "This is a compulsory header"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/100/sub/14", headers, "q1=12")

    headers = {}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/1000/sub/14", headers, "q1=12")

    headers = {"h1": "This is a compulsory header"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/100/sub/14", headers, "q2=12")

    headers = {"h1": "This is a compulsory header"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/100/sub/14", headers, "")

    headers = {"h1": "This is a compulsory header"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/100.1/sub/14", headers, "q1=1456434ff2")

    headers = {"h1": "This is  a compulsory header"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/no_mean/sub/14", headers, "q1=5645612")

    headers = {"h1": "This is a compulsory header"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1//sub/14", headers, "q1=12")
    end_test()


def test_http_validation_2(init_and_configure):
    command_list = []
    [validator_id_1, _add_api_operation_validator1] = ConfigurationSDK.add_api_operation_validator("api_1", "path_1",
                                                                                                   "GET")
    command_list += _add_api_operation_validator1
    command_list += ConfigurationSDK.add_data_type_header_validator(validator_id_1, "h1", "integer")
    command_list += ConfigurationSDK.add_data_type_path_validator(validator_id_1, "id1", "integer")
    command_list += ConfigurationSDK.add_data_type_query_validator(validator_id_1, "q1", "integer")
    GatewayProcess.configure_low_level(command_list)

    headers = {"h1": "100", "h2": "This is not a compulsory header"}
    decoded_response1 = GatewayProcess.http_get("localhost:4000", "/test_1/100/sub/14", headers, "q1=12")
    assert decoded_response1 == "Hello world"

    headers = {"h1": "100000000000000000000000000000000000000", "h2": "789"}
    decoded_response1 = GatewayProcess.http_get("localhost:4000", "/test_1/99999/sub/14", headers, "q1=0")
    assert decoded_response1 == "Hello world"

    headers = {"h1": "0x52", "h2": "789"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/99999/sub/14", headers, "q1=01")

    headers = {"h1": "1.00000000000000000000000000000000000000", "h2": "789"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/99999/sub/14", headers, "q1=0")

    headers = {"h1": "string"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/100/sub/14", headers, "q1=12")

    headers = {"h1": 'char'}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/100/sub/14", headers, "q1=12")

    headers = {"h1": True}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/100/sub/14", headers, "q1=12")

    headers = {"h1": 100}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/100.888888/sub/14", headers, "q1=12")

    headers = {"h1": "100", "h2": "This is not a compulsory header"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/0x45/sub/14", headers, "q1=12")

    headers = {"h1": "100", "h2": "This is not a compulsory header"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/string/sub/14", headers, "q1=12")

    headers = {"h1": "100", "h2": "This is not a compulsory header"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/True/sub/14", headers, "q1=12")

    headers = {"h1": "100", "h2": "This is not a compulsory header"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/100/sub/14", headers,
                                "q1=12.000000000007")

    headers = {"h1": "100", "h2": "This is not a compulsory header"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/100/sub/14", headers,
                                "q1=garbage_string")

    headers = {"h1": "100", "h2": "This is not a compulsory header"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/100/sub/14", headers, "q1=0x18")

    headers = {"h1": "100", "h2": "This is not a compulsory header"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/100/sub/14", headers, "q1=True")

    WiremockProcess.shutdown(8080)
    WiremockProcess.shutdown(8081)
    GatewayProcess.stop()


def test_http_validation_3(init_and_configure):
    command_list = []
    [validator_id_1, _add_api_operation_validator1] = ConfigurationSDK.add_api_operation_validator("api_1", "path_1",
                                                                                                   "GET")
    command_list += _add_api_operation_validator1
    command_list += ConfigurationSDK.add_compulsory_header_validator(validator_id_1, "h1")
    command_list += ConfigurationSDK.add_compulsory_header_validator(validator_id_1, "h2")
    command_list += ConfigurationSDK.add_data_type_header_validator(validator_id_1, "h3", "integer")
    command_list += ConfigurationSDK.add_data_type_header_validator(validator_id_1, "h4", "decimal")

    command_list += ConfigurationSDK.add_data_type_path_validator(validator_id_1, "id1", "integer")
    command_list += ConfigurationSDK.add_data_type_path_validator(validator_id_1, "id2", "decimal")

    command_list += ConfigurationSDK.add_compulsory_query_validator(validator_id_1, "q1")
    command_list += ConfigurationSDK.add_compulsory_query_validator(validator_id_1, "q2")
    command_list += ConfigurationSDK.add_data_type_query_validator(validator_id_1, "q3", "integer")
    command_list += ConfigurationSDK.add_data_type_query_validator(validator_id_1, "q4", "decimal")
    GatewayProcess.configure_low_level(command_list)

    headers = {"h1": "This is a compulsory header", "h2": "This is not a compulsory header", "h3": "27000",
               "h4": "69.00001"}
    decoded_response1 = GatewayProcess.http_get("localhost:4000", "/test_1/100/sub/0.03", headers,
                                                "q1=12&q2=p&q3=1000&q4=25.999")
    assert decoded_response1 == "Hello world"

    headers = {"h1": "This is a compulsory header", "h2": "This is a compulsory header", "h3": "27000",
               "h4": "6900001"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/100/sub/0.03", headers,
                                "q1=12&q2=p&q3=1000&q4=25.999")

    headers = {"h1": "This is a compulsory header", "h2": "This is a compulsory header", "h3": "27000.7",
               "h4": "69.00001"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/100/sub/0.03", headers,
                                "q1=12&q2=p&q3=1000&q4=25.999")

    headers = {"h1": "This is a compulsory header", "h3": "270007",
               "h4": "69.00001"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/100/sub/0.03", headers,
                                "q1=12&q2=p&q3=1000&q4=25.999")

    headers = {"h2": "This is a compulsory header", "h3": "27000"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/100/sub/0.03", headers,
                                "q1=12&q2=p&q3=1000&q4=25.999")

    headers = {}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/100/sub/0.03", headers,
                                "q1=12&q2=p&q3=1000&q4=25.999")

    headers = {"h1": "This is a compulsory header", "h2": "This is a compulsory header", "h3": "27000",
               "h4": "690000.1"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/100/sub/0.03", headers,
                                "q1=12&q2=p&q3=1000&q4=25999")

    headers = {"h1": "This is a compulsory header", "h2": "This is a compulsory header", "h3": "27000",
               "h4": "690000.1"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/100/sub/0.03", headers,
                                "q1=12&q2=p&q3=0.1000&q4=25.999")

    headers = {"h1": "This is a compulsory header", "h2": "This is a compulsory header", "h3": "27000",
               "h4": "690000.1"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/100/sub/0.03", headers,
                                "q1=12&q3=1000&q4=25.999")

    headers = {"h1": "This is a compulsory header", "h2": "This is a compulsory header", "h3": "27000",
               "h4": "690000.1"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/100/sub/0.03", headers,
                                "q3=1000&q4=25.999")

    headers = {"h1": "This is a compulsory header", "h2": "This is a compulsory header", "h3": "27000",
               "h4": "690000.1"}
    decoded_response1 = GatewayProcess.http_get("localhost:4000", "/test_1/100/sub/0.03", headers,
                                                "q1=12&q2=p&q4=25.999")
    assert decoded_response1 == "Hello world"

    headers = {"h1": "This is a compulsory header", "h2": "This is a compulsory header", "h3": "27000",
               "h4": "690000.1"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/100/sub/0.03", headers,
                                "")

    headers = {"h1": "This is a compulsory header", "h2": "This is a compulsory header", "h3": "27000",
               "h4": "690000.1"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/100/sub/0.03", headers,
                                "q1=12&q2=p&q3=1000&q4=test")

    headers = {"h1": "This is a compulsory header", "h2": "This is a compulsory header", "h3": "27000",
               "h4": "690000.1"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/100/sub/0.03", headers,
                                "q1=12&q2=p&q3=1000&q4=True")

    headers = {"h1": "This is a compulsory header", "h2": "This is a compulsory header", "h3": "27000",
               "h4": "690000.1"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/100/sub/0.03", headers,
                                "q1=12&q2=p&q3=test&q4=25.999")

    headers = {"h1": "This is a compulsory header", "h2": "This is a compulsory header", "h3": "27000",
               "h4": "690000.1"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/100/sub/3", headers,
                                "q1=12&q2=p&q3=1000&q4=25.999")

    headers = {"h1": "This is a compulsory header", "h2": "This is a compulsory header", "h3": "27000",
               "h4": "690000.1"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/100.8/sub/0.03", headers,
                                "q1=12&q2=p&q3=1000&q4=25.999")

    headers = {"h1": "This is a compulsory header", "h2": "This is a compulsory header", "h3": "27000",
               "h4": "690000.1"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/100/sub/test", headers,
                                "q1=12&q2=p&q3=1000&q4=25.999")

    headers = {"h1": "This is a compulsory header", "h2": "This is a compulsory header", "h3": "27000",
               "h4": "690000.1"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/test/sub/0.03", headers,
                                "q1=12&q2=p&q3=1000&q4=25.999")

    headers = {"h1": "This is a compulsory header", "h2": "This is a compulsory header", "h3": "27000",
               "h4": "690000.1"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/100/sub/True", headers,
                                "q1=12&q2=p&q3=1000&q4=25.999")

    headers = {"h1": "This is a compulsory header", "h2": "This is a compulsory header", "h3": "27000.7",
               "h4": "69.00001"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/100/sub/0.03", headers,
                                "q1=12&q2=p&q3=1000&q4=25.999")

    headers = {"h1": "1.00000000000000000000000000000000000000", "h2": "789"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/99999/sub/14", headers, "q1=0")

    headers = {"h1": "string"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/100/sub/14", headers, "q1=12")

    headers = {"h1": 'char'}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/100/sub/14", headers, "q1=12")

    headers = {"h1": True}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/100/sub/14", headers, "q1=12")

    headers = {"h1": 100}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/100.888888/sub/14", headers, "q1=12")

    headers = {"h1": "100", "h2": "This is not a compulsory header"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/0x45/sub/14", headers, "q1=12")

    headers = {"h1": "100", "h2": "This is not a compulsory header"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/string/sub/14", headers, "q1=12")

    headers = {"h1": "100", "h2": "This is not a compulsory header"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/True/sub/14", headers, "q1=12")

    headers = {"h1": "100", "h2": "This is not a compulsory header"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/100/sub/14", headers,
                                "q1=12.000000000007")

    headers = {"h1": "100", "h2": "This is not a compulsory header"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/100/sub/14", headers,
                                "q1=garbage_string")

    headers = {"h1": "100", "h2": "This is not a compulsory header"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/100/sub/14", headers, "q1=0x18")

    headers = {"h1": "100", "h2": "This is not a compulsory header"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1/100/sub/14", headers, "q1=True")

    end_test()
