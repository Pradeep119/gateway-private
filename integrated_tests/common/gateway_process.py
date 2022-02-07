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


import http.client
import os
import socket
import subprocess
import time

from common import http_requests


def is_alive():
    try:
        subprocess.check_output(["pidof", "gateway"])
        return True
    except subprocess.CalledProcessError:
        return False


class GatewayProcess:
    @staticmethod
    def start(path="~/development/axp/nextgen/gateway/cmake-build-debug/./gateway", arg1=4000, arg2=2500):
        execution_cmd = path + " " + str(arg1) + " " + str(arg2) + " core pthread " \
                                                                   "uSockets z ssl crypto dl opentelemetry_common opentelemetry_version opentelemetry_resources " \
                                                                   "opentelemetry_proto opentelemetry_trace opentelemetry_otlp_recordable " \
                                                                   "opentelemetry_exporter_ostream_span opentelemetry_zpages opentelemetry_exporter_otlp_grpc " \
                                                                   "opentelemetry_exporter_otlp_http http_client_curl opentelemetry_metrics " \
                                                                   "opentelemetry_exporter_ostream_metrics  "
        subprocess.Popen([execution_cmd + "1>/dev/null"], shell=True)

    @staticmethod
    def stop():
        if is_alive():
            pid_gateway = subprocess.check_output(["pidof", "gateway"])
            pid_list = str(pid_gateway).replace("\\n\'", "").replace("b\'", "").split(" ")
            for ite in pid_list:
                os.system("kill -9 " + str(ite))

    @staticmethod
    def configure(path=""):
        os.system("java -jar " + path)

    @staticmethod
    def configure_low_level(commands=[], port=5000):
        while True:
            if is_alive():
                break
            else:
                GatewayProcess.start()
            time.sleep(0.005)
        socket_connection = socket.socket()
        while True:
            if socket_connector(socket_connection, "localhost", port):
                break
            time.sleep(0.005)
        for command in commands:
            socket_connection.send((command + "\r").encode())
        socket_connection.close()
        time.sleep(0.1)

    @staticmethod
    def http_get(url, endpoint, headers={}, queries="", timeout=0.5):
        start_time = time.perf_counter()
        while True:
            if time.perf_counter() - start_time > timeout:
                raise socket.timeout
            if queries != "":
                data = http_connector(url, endpoint + "?" + queries, headers)
            else:
                data = http_connector(url, endpoint, headers)
            if data:
                return data
            time.sleep(0.005)


def http_connector(url, endpoint, headers, timeout=1):
    try:
        return http_requests.http_get(url, endpoint, headers, timeout)
    except socket.timeout:
        return False
    except http.client.RemoteDisconnected:
        return False


def socket_connector(socket_connection, url, port):
    try:
        socket_connection.connect((url, port))
        return True
    except ConnectionRefusedError:
        return False
