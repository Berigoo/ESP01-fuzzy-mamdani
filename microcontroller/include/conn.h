//
// Created by pach on 4/18/24.
//

#ifndef ESP_CONN_H
#define ESP_CONN_H

#endif //ESP_CONN_H

#include "./storageHandler.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>

#define INSECURE_HTTPS true

class conn{
public:
    static esp8266webserver::ESP8266WebServerTemplate<WiFiServer> server;

    static String mainServerHostname;

    static WiFiClientSecure wifiClient;
    static HTTPClient httpClient;

    static int postRequest(String contentType, String endpoint, String data, const char** collectHeaders, char** headersOut, int headerCount);

    static void openServer(bool& stop);
    static void closeServer();

    static IPAddress* localIp;
    static IPAddress* gateaway;
    static IPAddress* subnet;
    static const char* SSID;
};


