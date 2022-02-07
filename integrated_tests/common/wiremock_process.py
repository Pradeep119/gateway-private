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


import os
import subprocess
import time

from common import http_requests


def is_alive(port_number):
    try:
        http_requests.http_get("localhost:" + str(port_number), "/__admin", {})
        return True
    except BaseException:
        return False


class WiremockProcess:
    global wiremock_pid
    wiremock_pid = []

    @staticmethod
    def start(port_number):
        if is_alive(port_number):
            WiremockProcess.shutdown(port_number)
        command = "java  -jar ~/development/axp/nextgen/wiremock/wiremock-jre8-standalone-2.31.0.jar --port " + str(
            port_number) + " --disable-banner = true 1>/dev/null"
        wiremock_pid.append(str(subprocess.Popen([command], shell=True).pid + 1))
        while True:
            if is_alive(port_number):
                break
            time.sleep(0.005)

    @staticmethod
    def configurator(port_number, data):
        while True:
            if is_alive(port_number):
                break
            time.sleep(0.005)
        http_requests.http_post("localhost:" + str(port_number), "/__admin/mappings", data)

    @staticmethod
    def shutdown(port_number):
        if is_alive(port_number):
            http_requests.http_post("localhost:" + str(port_number), "/__admin/shutdown", "")
        time.sleep(0.1)
        if is_alive(port_number):
            pid_list = wiremock_pid
            if len(pid_list):
                for ite in pid_list:
                    os.system("kill -9 " + str(ite))
