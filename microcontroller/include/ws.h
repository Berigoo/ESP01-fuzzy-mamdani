//
// Created by pach on 4/24/24.
//

#ifndef ESP_WS_H
#define ESP_WS_H

#include <ESPAsyncTCP.h>
#include <ArduinoJson.h>
#include <WebSocketsClient.h>

class ws{
public:
    static WebSocketsClient client;
    static String hostname;
    static bool isConnected;

    static void events(WStype_t type, uint8_t * payload, size_t length);

    static void connect(String hostname, String extraHeaders);
    static void loop();
};

#endif //ESP_WS_H
