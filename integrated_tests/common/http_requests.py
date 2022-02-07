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


def http_get(url, endpoint, headers={}, timeout=1):
    connection = http.client.HTTPConnection(url, timeout=timeout)
    connection.request("GET", endpoint, "", headers)
    response = connection.getresponse()
    decoded_data = response.read().decode()
    connection.close()
    return decoded_data


def http_post(url, endpoint, data):
    connection = http.client.HTTPConnection(url)
    connection.request("POST", endpoint, data)
    response = connection.getresponse()
    connection.close()
    return response.read().decode()
