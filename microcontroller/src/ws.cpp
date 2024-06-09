//
// Created by pach on 4/24/24.
//
#include "../include/ws.h"

WebSocketsClient ws::client;
bool ws::isConnected = false;

void ws::events(WStype_t type, uint8_t * payload, size_t length) {
    String msg;
    JsonDocument doc;
    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[WSc] Disconnected!\n");
            ws::isConnected = false;
            break;
        case WStype_CONNECTED:
            Serial.printf("[WSc] Connected to url: %s\n", payload);
            ws::isConnected = true;
            break;
        case WStype_TEXT: {
            for (size_t i = 0; i < length; i++) {
                msg += (char) payload[i];
            }

            deserializeJson(doc, msg.c_str());
            break;
        }
        case WStype_BIN:
            Serial.printf("[WSc] get binary length: %u\n", length);

            hexdump(payload, length);
            break;
    }
}

void ws::connect(String hostname, String extraHeaders) {
    String host = hostname;
    int port = 8765;
    String endpoint = "/";

    int pos = hostname.indexOf(':');
    int pos2 = hostname.indexOf('/');
    if(pos != -1) {
        if(pos2 != -1) {
            port = (hostname.substring(pos + 1, pos2)).toInt();
        }else{
            port = (hostname.substring(pos + 1)).toInt();
        }

        host = hostname.substring(0, pos);
    }
    if(pos2 != -1)
        endpoint = hostname.substring(pos2);


    Serial.printf("Connecting to ws: %s %i %s \n", host.c_str(), port, endpoint.c_str());
    ws::client.begin(host, port, endpoint);
    if(extraHeaders != " "){
        ws::client.setExtraHeaders(extraHeaders.c_str());
        Serial.printf("with extra headers: %s\n", extraHeaders.c_str());
    }
    ws::client.onEvent(ws::events);
    ws::client.setReconnectInterval(5000);
}

void ws::loop() {
    client.loop();
}

