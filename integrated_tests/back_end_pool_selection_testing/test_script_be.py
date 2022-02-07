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
import pytest

from common.configuration_sdk import ConfigurationSDK
from common.gateway_process import GatewayProcess
from common.wiremock_process import WiremockProcess

# artifact Paths
Gateway_artifact_path = "~/development/axp/nextgen/gateway/cmake-build-debug/./gateway"
arg1 = 4000
arg2 = 2500


@pytest.fixture(scope="function")
def init():
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


def end_test():
    WiremockProcess.shutdown(8080)
    WiremockProcess.shutdown(8081)
    GatewayProcess.stop()


def test_backend_pool_1(init):
    command_list = []
    command_list += ConfigurationSDK.add_api("api_1", "pet store", "sample per store api", "HTTP")
    command_list += ConfigurationSDK.add_api_operation("api_1", "path_1", "GET", "/test_1")
    command_list += ConfigurationSDK.add_api_operation("api_1", "path_2", "GET", "/test_2")

    [pool_key, _add_backend_pool_list] = ConfigurationSDK.add_api_back_end_pool_api_scope("api_1", "REST", "HTTP")
    command_list += _add_backend_pool_list
    command_list += ConfigurationSDK.add_back_end("be_1", pool_key, "localhost", 8080)

    [pool_key, _add_backend_pool_list] = ConfigurationSDK.add_api_back_end_pool_path_scope("api_1", "path_2", "REST",
                                                                                           "HTTP")
    command_list += _add_backend_pool_list
    command_list += ConfigurationSDK.add_back_end("be_2", pool_key, "localhost", 8081)

    GatewayProcess.configure_low_level(command_list)

    headers = {"User-Agent": "user_agent_1"}
    decoded_response = GatewayProcess.http_get("localhost:4000", "/test_1", headers)
    assert decoded_response == "Hello world"

    headers = {"User-Agent": "user_agent_1"}
    decoded_response = GatewayProcess.http_get("localhost:4000", "/test_2", headers)
    assert decoded_response == "Hello world"

    end_test()


def test_backend_pool_2(init):
    command_list = []
    command_list += ConfigurationSDK.add_api("api_1", "pet store", "sample per store api", "HTTP")
    command_list += ConfigurationSDK.add_api_operation("api_1", "path_1", "GET", "/test_1")
    command_list += ConfigurationSDK.add_api_operation("api_1", "path_2", "GET", "/test_2")

    [pool_key, _add_backend_pool_list] = ConfigurationSDK.add_api_back_end_pool_api_scope("api_1", "REST",
                                                                                          "HTTP")
    command_list += _add_backend_pool_list
    command_list += ConfigurationSDK.add_back_end("be_1", pool_key, "localhost", 8080)
    command_list += ConfigurationSDK.add_back_end("be_2", pool_key, "localhost", 8081)

    [pool_key, _add_backend_pool_list] = ConfigurationSDK.add_api_back_end_pool_method_scope("api_1", "path_2", "GET",
                                                                                             "REST", "HTTP")
    command_list += _add_backend_pool_list
    command_list += ConfigurationSDK.add_back_end("be_2", pool_key, "localhost", 8081)

    GatewayProcess.configure_low_level(command_list)

    headers = {"User-Agent": "user_agent_1"}
    decoded_response = GatewayProcess.http_get("localhost:4000", "/test_1", headers)
    assert decoded_response == "Hello world"

    headers = {"User-Agent": "user_agent_1"}
    decoded_response = GatewayProcess.http_get("localhost:4000", "/test_2", headers)
    assert decoded_response == "Hello world"

    end_test()


def test_backend_pool_3(init):
    command_list = []
    command_list += ConfigurationSDK.add_api("api_1", "pet store", "sample per store api", "HTTP")
    command_list += ConfigurationSDK.add_api_operation("api_1", "path_1", "GET", "/test_1")
    command_list += ConfigurationSDK.add_api_operation("api_1", "path_2", "GET", "/test_2")

    [pool_key, _add_backend_pool_list] = ConfigurationSDK.add_api_back_end_pool_api_scope("api_1", "REST", "HTTPS")
    command_list += _add_backend_pool_list
    command_list += ConfigurationSDK.add_back_end("be_1", pool_key, "localhost", 8080)

    [pool_key, _add_backend_pool_list] = ConfigurationSDK.add_api_back_end_pool_path_scope("api_1", "path_2", "REST",
                                                                                           "HTTPS")
    command_list += _add_backend_pool_list
    command_list += ConfigurationSDK.add_back_end("be_2", pool_key, "localhost", 8081)

    GatewayProcess.configure_low_level(command_list)

    headers = {"User-Agent": "user_agent_1"}
    decoded_response = GatewayProcess.http_get("localhost:4000", "/test_1", headers)
    assert decoded_response == "Hello world"

    headers = {"User-Agent": "user_agent_1"}
    decoded_response = GatewayProcess.http_get("localhost:4000", "/test_2", headers)
    assert decoded_response == "Hello world"

    end_test()


def test_backend_pool_4(init):
    command_list = []
    command_list += ConfigurationSDK.add_api("api_1", "pet store", "sample per store api", "HTTP")
    command_list += ConfigurationSDK.add_api_operation("api_1", "path_1", "GET", "/test_1")
    command_list += ConfigurationSDK.add_api_operation("api_1", "path_2", "GET", "/test_2")

    [pool_key, _add_backend_pool_list] = ConfigurationSDK.add_api_back_end_pool_api_scope("api_1", "REST",
                                                                                          "HTTPS")
    command_list += _add_backend_pool_list
    command_list += ConfigurationSDK.add_back_end("be_1", pool_key, "localhost", 8080)
    command_list += ConfigurationSDK.add_back_end("be_2", pool_key, "localhost", 8081)

    [pool_key, _add_backend_pool_list] = ConfigurationSDK.add_api_back_end_pool_method_scope("api_1", "path_2", "GET",
                                                                                             "REST", "HTTPS")
    command_list += _add_backend_pool_list
    command_list += ConfigurationSDK.add_back_end("be_2", pool_key, "localhost", 8081)

    GatewayProcess.configure_low_level(command_list)

    headers = {"User-Agent": "user_agent_1"}
    decoded_response = GatewayProcess.http_get("localhost:4000", "/test_1", headers)
    assert decoded_response == "Hello world"

    headers = {"User-Agent": "user_agent_1"}
    decoded_response = GatewayProcess.http_get("localhost:4000", "/test_2", headers)
    assert decoded_response == "Hello world"

    end_test()
