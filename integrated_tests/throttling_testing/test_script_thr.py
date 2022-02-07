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
import time

import pytest

from common import http_requests
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
    command_list += ConfigurationSDK.add_api_operation("api_1", "path_1", "GET", "/test_1")
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


def dos(count, resource_path, timeout=0.5):
    # This function can return the time-spent for the dos attack
    start_time = time.perf_counter()
    for _ in range(count):
        decoded_response = http_requests.http_get("localhost:4000", resource_path, timeout=timeout)
        # assert decoded_response == "Hello world"
    end_time = time.perf_counter()
    return count/(end_time - start_time)


def test_throttling_1(init_and_configure):
    t1 = dos(200, "/test_1")
    command_list = []
    command_list += ConfigurationSDK.add_throttler_api_scope("api_1","sliding_log", 10, 100)
    GatewayProcess.configure_low_level(command_list)
    t2 = dos(200, "/test_1")
    t3 = dos(20000, "/test_1")

    print(t1,t2,t3)
    command_list = []
    command_list += ConfigurationSDK.add_throttler_path_scope("api_1", "path_2", "sliding_log", 1000, 300)
    GatewayProcess.configure_low_level(command_list)
    dos(2000, "/test_2")

    end_test()


def test_throttling_2(init_and_configure):
    dos(200, "/test_1")
    command_list = []
    command_list += ConfigurationSDK.add_throttler_path_scope("api_1", "path_1", "sliding_log", 1, 1)
    GatewayProcess.configure_low_level(command_list)
    dos(200, "/test_1")

    dos(2000, "/test_2")
    command_list = []
    command_list += ConfigurationSDK.add_throttler_path_scope("api_1", "path_2", "sliding_log", 100, 5)
    GatewayProcess.configure_low_level(command_list)
    dos(2000, "/test_2")

    end_test()

# def test_throttling_3():
#     init_and_configure(throttling_sdk_artifact_path)
#
#     dos(50000, "/test_1")
#     command_list = []
#     command_list += ConfigurationSDK.add_throttler_path_scope("api_1", "path_1", "sliding_log", 1, 1)
#     GatewayProcess.configure_low_level(command_list)
#     with pytest.raises(socket.timeout):
#         dos(50000, "/test_1",0.1)
#
#     dos(2000, "/test_2")
#     command_list = []
#     command_list += ConfigurationSDK.add_throttler_path_scope("api_1", "path_2", "sliding_log", 100, 5)
#     GatewayProcess.configure_low_level(command_list)
#     dos(2000, "/test_2")
#
#     end_test()
