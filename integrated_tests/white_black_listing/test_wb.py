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

from common import http_requests
from common.configuration_sdk import ConfigurationSDK, BWListProperty
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
    command_list += ConfigurationSDK.add_throttler_path_scope("api_1", "path_1", "sliding_log", 1000, 250)
    command_list += ConfigurationSDK.add_throttler_path_scope("api_1", "path_2", "sliding_log", 1000, 250)
    command_list += ConfigurationSDK.add_throttler_api_scope("api_2", "sliding_log", 1000, 250)
    GatewayProcess.configure_low_level(command_list)


def end_test():
    WiremockProcess.shutdown(8080)
    WiremockProcess.shutdown(8081)
    GatewayProcess.stop()


def test_no_whitelist_blacklist_test(init_and_configure):
    headers = {"user-Agent": "user_agent_1"}
    decoded_response1 = GatewayProcess.http_get("localhost:4000", "/test_1", headers)
    assert decoded_response1 == "Hello world"

    headers = {"No-user-Agent": "garbage_data"}
    decoded_response1 = GatewayProcess.http_get("localhost:4000", "/test_1", headers)
    assert decoded_response1 == "Hello world"
    end_test()


def test_whitelist_only_sender_ip_api_scope(init_and_configure):
    command_list = []
    command_list += ConfigurationSDK.add_to_white_list_api_scope("api_1", "0000:0000:0000:0000:0000:0000:0000:0001",
                                                                 BWListProperty.SENDER_IP_ADDRESS)
    GatewayProcess.configure_low_level(command_list)

    headers = {"User-Agent": "user_agent_1"}
    decoded_response = GatewayProcess.http_get("localhost:4000", "/test_1", headers)
    assert decoded_response == "Hello world"
    decoded_response1 = GatewayProcess.http_get("localhost:4000", "/test_2", headers)
    assert decoded_response1 == "Hello world"
    end_test()


def test_whitelist_only_sender_ip_path_scope(init_and_configure):
    command_list = []
    command_list += ConfigurationSDK.add_to_white_list_path_scope("api_1", "path_1",
                                                                  "0000:0000:0000:0000:0000:0000:0000:0001",
                                                                  BWListProperty.SENDER_IP_ADDRESS)
    GatewayProcess.configure_low_level(command_list)

    headers = {"User-Agent": "user_agent_1"}
    decoded_response = GatewayProcess.http_get("localhost:4000", "/test_1", headers)
    assert decoded_response == "Hello world"
    decoded_response1 = GatewayProcess.http_get("localhost:4000", "/test_2", headers)
    assert decoded_response1 == "Hello world"
    end_test()


def test_whitelist_only_sender_ip_method_scope(init_and_configure):
    command_list = []
    command_list += ConfigurationSDK.add_to_white_list_method_scope("api_1", "path_1", "GET",
                                                                    "0000:0000:0000:0000:0000:0000:0000:0001",
                                                                    BWListProperty.SENDER_IP_ADDRESS)
    GatewayProcess.configure_low_level(command_list)

    headers = {"User-Agent": "user_agent_1"}
    decoded_response = GatewayProcess.http_get("localhost:4000", "/test_1", headers)
    assert decoded_response == "Hello world"
    decoded_response1 = GatewayProcess.http_get("localhost:4000", "/test_2", headers)
    assert decoded_response1 == "Hello world"
    end_test()


def test_whitelist_only_user_agent_api_scope(init_and_configure):
    command_list = []
    command_list += ConfigurationSDK.add_to_white_list_api_scope("api_1", "user_agent_1",
                                                                 BWListProperty.SENDER_USER_AGENT)
    GatewayProcess.configure_low_level(command_list)

    headers = {"User-Agent": "user_agent_1"}
    decoded_response = GatewayProcess.http_get("localhost:4000", "/test_1", headers)
    assert decoded_response == "Hello world"
    decoded_response = GatewayProcess.http_get("localhost:4000", "/test_2", headers)
    assert decoded_response == "Hello world"
    end_test()


def test_whitelist_only_user_agent_path_scope(init_and_configure):
    command_list = []
    command_list += ConfigurationSDK.add_to_white_list_path_scope("api_1", "path_1", "user_agent_1",
                                                                  BWListProperty.SENDER_USER_AGENT)
    GatewayProcess.configure_low_level(command_list)

    headers = {"User-Agent": "user_agent_1"}
    decoded_response = GatewayProcess.http_get("localhost:4000", "/test_1", headers)
    assert decoded_response == "Hello world"
    decoded_response = GatewayProcess.http_get("localhost:4000", "/test_2", headers)
    assert decoded_response == "Hello world"
    end_test()


def test_whitelist_only_user_agent_method_scope(init_and_configure):
    command_list = []
    command_list += ConfigurationSDK.add_to_white_list_method_scope("api_1", "path_1", "GET", "user_agent_1",
                                                                    BWListProperty.SENDER_USER_AGENT)
    GatewayProcess.configure_low_level(command_list)

    headers = {"User-Agent": "user_agent_1"}
    decoded_response = GatewayProcess.http_get("localhost:4000", "/test_1", headers)
    assert decoded_response == "Hello world"
    decoded_response1 = GatewayProcess.http_get("localhost:4000", "/test_2", headers)
    assert decoded_response1 == "Hello world"
    end_test()


def test_whitelist_api_scope(init_and_configure):
    command_list = []
    command_list += ConfigurationSDK.add_to_white_list_api_scope("api_1", "0000:0000:0000:0000:0000:0000:0000:0001",
                                                                 BWListProperty.SENDER_IP_ADDRESS)
    command_list += ConfigurationSDK.add_to_white_list_api_scope("api_1", "user_agent_1",
                                                                 BWListProperty.SENDER_USER_AGENT)
    command_list += ConfigurationSDK.add_to_white_list_api_scope("api_1", "user_agent_5",
                                                                 BWListProperty.SENDER_USER_AGENT)
    GatewayProcess.configure_low_level(command_list)

    headers = {"User-Agent": "user_agent_1"}
    decoded_response = GatewayProcess.http_get("localhost:4000", "/test_1", headers)
    assert decoded_response == "Hello world"

    headers = {"User-Agent": "user_agent_2"}
    with pytest.raises(socket.timeout):
        http_requests.http_get("localhost:4000", "/test_1", headers)

    end_test()


def test_whitelist_path_scope(init_and_configure):
    command_list = []
    command_list += ConfigurationSDK.add_to_white_list_path_scope("api_1", "path_1",
                                                                  "0000:0000:0000:0000:0000:0000:0000:0001",
                                                                  BWListProperty.SENDER_IP_ADDRESS)
    command_list += ConfigurationSDK.add_to_white_list_path_scope("api_1", "path_1", "user_agent_1",
                                                                  BWListProperty.SENDER_USER_AGENT)
    command_list += ConfigurationSDK.add_to_white_list_path_scope("api_1", "path_1", "user_agent_5",
                                                                  BWListProperty.SENDER_USER_AGENT)
    GatewayProcess.configure_low_level(command_list)

    headers = {"User-Agent": "user_agent_1"}
    decoded_response = GatewayProcess.http_get("localhost:4000", "/test_1", headers)
    assert decoded_response == "Hello world"

    headers = {"User-Agent": "user_agent_2"}
    with pytest.raises(socket.timeout):
        http_requests.http_get("localhost:4000", "/test_1", headers)

    end_test()


def test_whitelist_method_scope(init_and_configure):
    command_list = []
    command_list += ConfigurationSDK.add_to_white_list_method_scope("api_1", "path_1", "GET",
                                                                    "0000:0000:0000:0000:0000:0000:0000:0001",
                                                                    BWListProperty.SENDER_IP_ADDRESS)
    command_list += ConfigurationSDK.add_to_white_list_method_scope("api_1", "path_1", "GET", "user_agent_1",
                                                                    BWListProperty.SENDER_USER_AGENT)
    command_list += ConfigurationSDK.add_to_white_list_method_scope("api_1", "path_1", "GET", "user_agent_5",
                                                                    BWListProperty.SENDER_USER_AGENT)
    GatewayProcess.configure_low_level(command_list)

    headers = {"User-Agent": "user_agent_1"}
    decoded_response = GatewayProcess.http_get("localhost:4000", "/test_1", headers)
    assert decoded_response == "Hello world"

    headers = {"User-Agent": "user_agent_2"}
    with pytest.raises(socket.timeout):
        http_requests.http_get("localhost:4000", "/test_1", headers)

    end_test()


def test_blacklist_only_sender_ip(init_and_configure):
    command_list = []
    command_list += ConfigurationSDK.add_to_black_list_api_scope("api_1", "0000:0000:0000:0000:0000:0000:0000:0001",
                                                                 BWListProperty.SENDER_IP_ADDRESS)
    GatewayProcess.configure_low_level(command_list)

    headers = {"User-Agent": "user_agent_1"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1", headers)
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_2", headers)
    end_test()


def test_blacklist_only_user_agent(init_and_configure):
    command_list = []
    command_list += ConfigurationSDK.add_to_black_list_api_scope("api_1", "user_agent_1",
                                                                 BWListProperty.SENDER_USER_AGENT)
    GatewayProcess.configure_low_level(command_list)

    headers = {"User-Agent": "user_agent_1"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1", headers)
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_2", headers)

    headers = {}
    decoded_response = GatewayProcess.http_get("localhost:4000", "/test_1", headers)
    assert decoded_response == "Hello world"

    headers = {"User-Agent": "user_agent_2"}
    decoded_response = GatewayProcess.http_get("localhost:4000", "/test_1", headers)
    assert decoded_response == "Hello world"
    end_test()


def test_white_black_list_comb_1(init_and_configure):
    command_list = []
    command_list += ConfigurationSDK.add_to_white_list_path_scope("api_1", "path_1", "user_agent_1",
                                                                  BWListProperty.SENDER_USER_AGENT)
    command_list += ConfigurationSDK.add_to_black_list_path_scope("api_1", "path_2", "user_agent_1",
                                                                  BWListProperty.SENDER_USER_AGENT)
    command_list += ConfigurationSDK.add_to_black_list_path_scope("api_1", "path_2", "user_agent_2",
                                                                  BWListProperty.SENDER_USER_AGENT)
    command_list += ConfigurationSDK.add_to_black_list_api_scope("api_1", "user_agent_3",
                                                                 BWListProperty.SENDER_USER_AGENT)

    command_list += ConfigurationSDK.add_to_white_list_api_scope("api_1", "0000:0000:0000:0000:0000:0000:0000:0001",
                                                                 BWListProperty.SENDER_IP_ADDRESS)
    GatewayProcess.configure_low_level(command_list)

    headers = {"User-Agent": "user_agent_1"}
    decoded_response = GatewayProcess.http_get("localhost:4000", "/test_1", headers)
    assert decoded_response == "Hello world"

    headers = {"User-Agent": "user_agent_1"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_2", headers)

    headers = {"User-Agent": "user_agent_2"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_2", headers)

    headers = {"User-Agent": "user_agent_2"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1", headers)

    headers = {"User-Agent": "user_agent_3"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1", headers)

    headers = {"User-Agent": "user_agent_3"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_2", headers)

    end_test()


def test_white_black_list_comb_2(init_and_configure):
    command_list = []
    command_list += ConfigurationSDK.add_to_white_list_api_scope("api_1", "user_agent_1",
                                                                 BWListProperty.SENDER_USER_AGENT)
    command_list += ConfigurationSDK.add_to_white_list_api_scope("api_1", "user_agent_2",
                                                                 BWListProperty.SENDER_USER_AGENT)
    command_list += ConfigurationSDK.add_to_white_list_api_scope("api_1", "0000:0000:0000:0000:0000:0000:0000:0001",
                                                                 BWListProperty.SENDER_IP_ADDRESS)

    command_list += ConfigurationSDK.add_to_black_list_path_scope("api_1", "path_2", "user_agent_1",
                                                                  BWListProperty.SENDER_USER_AGENT)
    command_list += ConfigurationSDK.add_to_black_list_path_scope("api_1", "path_2",
                                                                  "0000:0000:0000:0000:0000:0000:0000:0001",
                                                                  BWListProperty.SENDER_IP_ADDRESS)

    GatewayProcess.configure_low_level(command_list)

    headers = {"User-Agent": "user_agent_1"}
    decoded_response = GatewayProcess.http_get("localhost:4000", "/test_1", headers)
    assert decoded_response == "Hello world"

    headers = {"User-Agent": "user_agent_1"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_2", headers)

    headers = {"User-Agent": "user_agent_2"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_2", headers)

    headers = {"User-Agent": "user_agent_2"}
    decoded_response = GatewayProcess.http_get("localhost:4000", "/test_1", headers)
    assert decoded_response == "Hello world"

    headers = {"User-Agent": "user_agent_3"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1", headers)

    headers = {"User-Agent": "user_agent_3"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_2", headers)

    end_test()


def test_white_black_list_comb_3(init_and_configure):
    command_list = []
    command_list += ConfigurationSDK.add_to_white_list_path_scope("api_1", "path_1", "user_agent_1",
                                                                  BWListProperty.SENDER_USER_AGENT)
    command_list += ConfigurationSDK.add_to_white_list_path_scope("api_1", "path_2", "user_agent_2",
                                                                  BWListProperty.SENDER_USER_AGENT)
    command_list += ConfigurationSDK.add_to_white_list_path_scope("api_1", "path_1",
                                                                  "0000:0000:0000:0000:0000:0000:0000:0001",
                                                                  BWListProperty.SENDER_IP_ADDRESS)

    command_list += ConfigurationSDK.add_to_black_list_api_scope("api_1", "user_agent_1",
                                                                 BWListProperty.SENDER_USER_AGENT)
    command_list += ConfigurationSDK.add_to_black_list_api_scope("api_1",
                                                                 "0000:0000:0000:0000:0000:0000:0000:0001",
                                                                 BWListProperty.SENDER_IP_ADDRESS)

    GatewayProcess.configure_low_level(command_list)

    headers = {"User-Agent": "user_agent_1"}
    decoded_response = GatewayProcess.http_get("localhost:4000", "/test_1", headers)
    assert decoded_response == "Hello world"

    headers = {"User-Agent": "user_agent_1"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_2", headers)

    headers = {"User-Agent": "user_agent_2"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1", headers)

    headers = {"User-Agent": "user_agent_2"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_2", headers)

    headers = {"User-Agent": "user_agent_3"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1", headers)

    headers = {"User-Agent": "user_agent_3"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_2", headers)

    end_test()
