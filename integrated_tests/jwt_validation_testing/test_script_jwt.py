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
#      @author Braveenan Sritharan (braveenan.sritharan@axiatadigitallabs.com) on 2022/01/21.
#
#

import json
import socket

import pytest

from common.configuration_sdk import ConfigurationSDK
from common.gateway_process import GatewayProcess
from common.wiremock_process import WiremockProcess

# artifact Paths
Gateway_artifact_path = "~/development/axp/nextgen/gateway/cmake-build-debug/gateway"
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


def test_valid_jwt(init_and_configure):
    headers = {
        "authorization": "eyJ4NXQiOiJNell4TW1Ga09HWXdNV0kwWldObU5EY3hOR1l3WW1NNFpUQTNNV0kyTkRBelpHUXpOR00wWkdSbE5qSmtPREZrWkRSaU9URmtNV0ZoTXpVMlpHVmxOZyIsImtpZCI6Ik16WXhNbUZrT0dZd01XSTBaV05tTkRjeE5HWXdZbU00WlRBM01XSTJOREF6WkdRek5HTTBaR1JsTmpKa09ERmtaRFJpT1RGa01XRmhNelUyWkdWbE5nX1JTMjU2IiwiYWxnIjoiUlMyNTYifQ.eyJzdWIiOiJhZG1pbiIsImF1dCI6IkFQUExJQ0FUSU9OIiwiYXVkIjoiNDNSTllaZVh4YzJNaFpYZEdRX1djalBJb3FnYSIsIm5iZiI6MTY0MTgxODQ1MCwiYXpwIjoiNDNSTllaZVh4YzJNaFpYZEdRX1djalBJb3FnYSIsImlzcyI6Imh0dHBzOlwvXC9sb2NhbGhvc3Q6OTQ0M1wvb2F1dGgyXC90b2tlbiIsImV4cCI6MTY0NDQ5Njg1MCwiaWF0IjoxNjQxODE4NDUwLCJqdGkiOiI2NjZlMjEyYy1iMzM3LTRmMjEtODc2OC00NzRlMmU2NGFjZTUifQ.kFzG8w4lX7KK1i5qcppuj6-lGAWwiH-V-gsFbLOExtFaW7OnkYB8bdPoH8oPlYh5tR1QQnPdB-BwVQurHmxEykqQnUO-iwlJspxdPxd35xIBYiZNCqmAVme4j7PSb-Sub1bJeah4qhDMJ28foJJlgomdpNttraNaYpduY3EhUy8F39s8m-F1xAXxUArAeQL0YKI6l38aIxlUX6Vw492nzXCLpq7MOS3OA0bYbLqt-e9t5E_HK1NRRRfZ71wGcR1pzIocdGFPkPhjWqO5zKoM8DXRHTuCp5aaxPA55dHZAFlCDSZbIcEPWm2ILAMpNxntDXbiJPYIXMsPCGL_NJHOvw"}
    decoded_response1 = GatewayProcess.http_get("localhost:4000", "/test_1", headers)
    assert decoded_response1 == "Hello world"


def test_expired_jwt(init_and_configure):
    headers = {
        "authorization": "eyJ4NXQiOiJNell4TW1Ga09HWXdNV0kwWldObU5EY3hOR1l3WW1NNFpUQTNNV0kyTkRBelpHUXpOR00wWkdSbE5qSmtPREZrWkRSaU9URmtNV0ZoTXpVMlpHVmxOZyIsImtpZCI6Ik16WXhNbUZrT0dZd01XSTBaV05tTkRjeE5HWXdZbU00WlRBM01XSTJOREF6WkdRek5HTTBaR1JsTmpKa09ERmtaRFJpT1RGa01XRmhNelUyWkdWbE5nX1JTMjU2IiwiYWxnIjoiUlMyNTYifQ.eyJzdWIiOiJhZG1pbiIsImF1dCI6IkFQUExJQ0FUSU9OIiwiYXVkIjoieUFxbzh1WG1uM2dlelY4aVZqUTFJU1RTSmI4YSIsIm5iZiI6MTY0MTgxODY0OCwiYXpwIjoieUFxbzh1WG1uM2dlelY4aVZqUTFJU1RTSmI4YSIsImlzcyI6Imh0dHBzOlwvXC9sb2NhbGhvc3Q6OTQ0M1wvb2F1dGgyXC90b2tlbiIsImV4cCI6MTY0MTgxODcwOCwiaWF0IjoxNjQxODE4NjQ4LCJqdGkiOiIzYmZiZWNlZC1hMTE1LTQxZGQtYjFiZi0zZTI0MzBlNTM4NTgifQ.djfMED8ROrbh0YIjJK1TnHF6u3daCpnRMWzviUT3470FnPiWU1efp7RfehrPD4yaTKlulKyAa_W4EuumPdIIc9DdygOniABrSQamSIVDm39dbsZ9i4QgTuQGsB2Y8K8SgTNr82xnW_gWSoVpwMAsbjqwHE9XBP8ywMIvgZloKD2KRnHbscDyRaqvZjmrPe2yYUS4u6hPYRTcHdaN6Zl_qH7CLTNlRySeZ8csGcBsNO_d96XTn8UNOGYSZc35UQ3MiOxIo49jFNpwQkPdvxppf0gn9zutesEAauLCR-NlkrJXH9PKy1UcGfDc_-uKXd5t4DRyDL79zaxhnjp3gW-1Dg"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1", headers)


def test_non_jwt(init_and_configure):
    headers = {
        "authorization": "e3631361-f45a-39e6-9b76-d051619d0402"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1", headers)


def test_invalid_jwt(init_and_configure):
    headers = {
        "authorization": "eyJ4NXQiOiJNell4TW1Ga09HWXdNV0kwWldObU5EY3hOR1l3WW1NNFpUQTNNV0kyTkRBelpHUXpOR00wWkdSbE5qSmtPREZrWkRSaU9URmtNV0ZoTXpVMlpHVmxOZyIsImtpZCI6Ik16WXhNbUZrT0dZd01XSTBaV05tTkRjeE5HWXdZbU00WlRBM01XSTJOREF6WkdRek5HTTBaR1JsTmpKa09ERmtaRFJpT1RGa01XRmhNelUyWkdWbE5nX1JTMjU2IiwiYWxnIjoiUlMyNTYifQ.eyJzdWIiOiJhZG1pbiIsImF1dCI6IkFQUExJQ0FUSU9OIiwiYXVkIjoiNDNSTllaZVh4YzJNaFpYZEdRX1djalBJb3FnYSIsIm5iZiI6MTY0MDAwMjkyNywiYXpwIjoiNDNSTllaZVh4YzJNaFpYZEdRX1djalBJb3FnYSIsImlzcyI6Imh0dHBzOlwvXC9sb2NhbGhvc3Q6OTQ0M1wvb2F1dGgyXC90b2tlbiIsImV4cCI6MTY0MjY4MTMyNywiaWF0IjoxNjQwMDAyOTI3LCJqdGkiOiI1ZmI4MWFhOC0xOTBkLTQ5MWItYTIwMC0wZTA4YzA5YTc1MGQifQ.QcZ3PEa-C-I9rq5uZ_7B7HK6xQH8dQ3Rv7momX9CML_YTybvYhQ-jhoTP4bB7KU2SyW2pyQS3PXHnZov_SzU0-QhxzTZJGLEjRImo4W-o0YwI8k4S5t-kc5eRsx5cEQiFd5r-aeIsCrfyVEC-rFzuKWGx9j2VtLCVqab62ZXUpgnjEmLk0qL6An-yhVQtMiqrYrSs-QM3JYxBUZriiBgijMsmYBV1_15E5y-s55nDEXo4btRumPqdUyS-_pp2XzMdMEOqnYrTDsORYKOYBc_jEERx2aWs6EMUkjGozad518K1MO3EMh9cMul1iqWLN4jy7YVkUISei1o4LkCnmQ0yg"}
    with pytest.raises(socket.timeout):
        GatewayProcess.http_get("localhost:4000", "/test_1", headers)


def test_valid_jwt_2(init_and_configure):
    headers = {
        "authorization": "eyJ4NXQiOiJNell4TW1Ga09HWXdNV0kwWldObU5EY3hOR1l3WW1NNFpUQTNNV0kyTkRBelpHUXpOR00wWkdSbE5qSmtPREZrWkRSaU9URmtNV0ZoTXpVMlpHVmxOZyIsImtpZCI6Ik16WXhNbUZrT0dZd01XSTBaV05tTkRjeE5HWXdZbU00WlRBM01XSTJOREF6WkdRek5HTTBaR1JsTmpKa09ERmtaRFJpT1RGa01XRmhNelUyWkdWbE5nX1JTMjU2IiwiYWxnIjoiUlMyNTYifQ.eyJzdWIiOiJhZG1pbiIsImF1dCI6IkFQUExJQ0FUSU9OIiwiYXVkIjpbIlBzT01MRUdXc3I5bEJxNnFOcGI2VHI1Nm1oOGEiLCJhdWRfMSJdLCJuYmYiOjE2NDE4MTkwMDMsImF6cCI6IlBzT01MRUdXc3I5bEJxNnFOcGI2VHI1Nm1oOGEiLCJzY29wZSI6InJlYWRfYWNjZXNzIiwiaXNzIjoiaHR0cHM6XC9cL2xvY2FsaG9zdDo5NDQzXC9vYXV0aDJcL3Rva2VuIiwiZXhwIjoxNjczMzU1MDAzLCJpYXQiOjE2NDE4MTkwMDMsImp0aSI6IjY5MjM5MTNjLWI4NTctNDNhOS1iNjhiLTQ4NDE2OGQ5YjBiZCJ9.mSJeeL1eYKmbyPHxKHahWtKr789LcEw6K4m0fFDbOjNCa1j1jjI6lx4dlfQ1q7icTAe0FKw3BLpq31POZV9MdNyuQMUxpMKBdWhDf03Sk-UmK2CEl73qcLu8l6ao7deQ5RLl_IcCWBfrqF9IxO2rNs583SUSXYt3Ob2LA5nzKOcb6rmA-724zwxbvtIncOePJMdE6TRT68byb5m5bJgjL8wJHdcG_LHDDQFp-Ze3-MXzQ3yUpw4AkzfqiFWjYql8RXC1dh9KfMcOZ-HGRcTYc23M986AXCczkXRweZD3oYyaZP_CuhWiHHMGpKV8fQX5IWXJ0XJta3c6QNbUF9U5yw"}
    decoded_response1 = GatewayProcess.http_get("localhost:4000", "/test_1", headers)
    assert decoded_response1 == "Hello world"


def test_valid_jwt_3(init_and_configure):
    headers = {
        "authorization": "eyJ4NXQiOiJNell4TW1Ga09HWXdNV0kwWldObU5EY3hOR1l3WW1NNFpUQTNNV0kyTkRBelpHUXpOR00wWkdSbE5qSmtPREZrWkRSaU9URmtNV0ZoTXpVMlpHVmxOZyIsImtpZCI6Ik16WXhNbUZrT0dZd01XSTBaV05tTkRjeE5HWXdZbU00WlRBM01XSTJOREF6WkdRek5HTTBaR1JsTmpKa09ERmtaRFJpT1RGa01XRmhNelUyWkdWbE5nX1JTMjU2IiwiYWxnIjoiUlMyNTYifQ.eyJzdWIiOiJhZG1pbiIsImF1dCI6IkFQUExJQ0FUSU9OIiwiYXVkIjpbIkM3ZFV4b0VaZ0w4Z1g4WjZ6YUNTdzM3M1ZLOGEiLCJhdWRfMyIsImF1ZF8yIl0sIm5iZiI6MTY0MTgxOTE3NiwiYXpwIjoiQzdkVXhvRVpnTDhnWDhaNnphQ1N3MzczVks4YSIsInNjb3BlIjoicmVhZF9hY2Nlc3Mgd3JpdGVfYWNjZXNzIiwiaXNzIjoiaHR0cHM6XC9cL2xvY2FsaG9zdDo5NDQzXC9vYXV0aDJcL3Rva2VuIiwiZXhwIjoxNjczMzU1MTc2LCJpYXQiOjE2NDE4MTkxNzYsImp0aSI6ImQzNzZiOGEzLWQ0ODUtNDFmZS1iZjY5LTI4NmYzZjRhMWZmZCJ9.rPBgpxx_Ik2sm-fm1_k_KQMeIKix7Tls-NUP5Ze2C_qsRLPTBYQ_udInbXplBonASGvSHCHZ4qQJA3bDtYnyoUYY_wcJ-aQQVjCl_WL27ZBDeT9ltsQbDAABXpUXcY2dwrQiOXz0ANyBTrDy5H1RV_9WYr5-PPCtBYbnNCHl6TDbLCDrjoEJ7GKso6voQGo5uWmkK3NA06J1GrNwK6SmrKwV53mr7l1RkbQsQLffE-AIK_2PmrP1srrlNlFdWWTC4KxgMJnwDWFFBaGD3V_LJ-auNeywmGYCLVg_9Omp4Pop3bLXdO0Ia99e0Smq1OAmJi8HomLdN2MdyHtzxui1uw"}
    decoded_response1 = GatewayProcess.http_get("localhost:4000", "/test_1", headers)
    assert decoded_response1 == "Hello world"
