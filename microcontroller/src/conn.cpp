//
// Created by pach on 4/18/24.
//


#include "../include/conn.h"


ESP8266WebServer conn::server = ESP8266WebServer(80);
IPAddress* conn::localIp = new IPAddress(192, 168, 0, 2);
IPAddress* conn::gateaway = new IPAddress(192, 168, 0, 1);
IPAddress* conn::subnet = new IPAddress(255, 255, 255, 0);
const char* conn::SSID = "ESP_01";

WiFiClientSecure conn::wifiClient;
HTTPClient conn::httpClient;
String conn::mainServerHostname = "https://192.168.10.5:8000";

void conn::openServer(bool& stop) {

    WiFi.mode(WiFiMode_t::WIFI_AP);
    WiFi.softAPConfig(*conn::localIp, *conn::gateaway, *conn::subnet);
    WiFi.softAP(conn::SSID);

    server.on("/set", [&](){
        if (server.method() != HTTP_POST) {
            stop = false;
            server.send(405, "text/plain", "Method Not Allowed");
        } else {
            Serial.println("POST");
            if(server.hasArg("ssid") && server.hasArg("ssid_pass") && server.hasArg("hostname")){
                Serial.println("Setting data");

                String ssid = server.arg("ssid");
                String ssidPass = server.arg("ssid_pass");
                String hostname = server.arg("hostname");

                memcpy(&storage::ssid, ssid.c_str(), ssid.length()+1);                  // +1 for null terminator
                memcpy(&storage::ssid_pass, ssidPass.c_str(), ssidPass.length()+1);
                memcpy(&storage::hostname, hostname.c_str(), hostname.length()+1);

                Serial.println(ssid);
                Serial.println(ssidPass);

                if(storageHandler::write()){
                    stop = true;
                    server.send(200);
                }else{
                    stop = false;
                    server.send(500);
                }
            }else{
                stop = false;
                server.send(422);
            }
        }
    });

    server.begin();
}

void conn::closeServer() {
    server.close();
    WiFi.mode(WiFiMode_t::WIFI_STA);
    delay(100);
}

/// @warning dont forget to free 'headersOut'
/// @return https response code
/// @param headersOut reserve the memory first
int conn::postRequest(String contentType, String endpoint, String data, const char **collectHeaders,
                      char** headersOut, int headerCount) {
    String finalEndpoint = mainServerHostname;
    finalEndpoint.concat(endpoint);
    
    if(INSECURE_HTTPS) wifiClient.setInsecure();

    httpClient.begin(wifiClient, finalEndpoint);
    httpClient.addHeader("Content-Type", contentType);

    int ret;

    if (collectHeaders) {
        httpClient.collectHeaders(collectHeaders, headerCount);
        ret = httpClient.POST(data);
        for (int j = 0; j < httpClient.headers(); ++j) {
            const String holder = httpClient.header(j);
            headersOut[j] = (char*) malloc(holder.length());
            memcpy(headersOut[j], holder.c_str(), holder.length()+1);   // +1 for null terminator
        }
    } else {
        ret = httpClient.POST(data);
    }

    httpClient.end();
    return ret;
}