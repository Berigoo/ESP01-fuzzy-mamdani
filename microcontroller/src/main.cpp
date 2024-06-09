#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <DHT_U.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <ESP_EEPROM.h>

#include "../include/storageHandler.h"
#include "../include/ws.h"
#include "../include/conn.h"
#include "../include/conf.h"

#define DHTTYPE DHT11

#define TIMEOUT 10000

DHT_Unified dht11(DHT_11_PIN, DHTTYPE);
unsigned int delayMs = 2000;

void setup() {
    Serial.begin(115200);
    pinMode(DHT_11_PIN, INPUT);
    EEPROM.begin(SSID_LEN + PASS_LEN + HOSTNAME_LEN);

    if(READ_EEPROM) storageHandler::read();

    // dht sensor
    dht11.begin();
    sensor_t sensor;
    dht11.temperature().getSensor(&sensor);
    Serial.println("------------------------------------");
    Serial.println("Temperature");
    Serial.print  ("Sensor:       "); Serial.println(sensor.name);
    Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
    Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
    Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" *C");
    Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" *C");
    Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" *C");
    Serial.println("------------------------------------");
    dht11.humidity().getSensor(&sensor);
    Serial.println("------------------------------------");
    Serial.println("Humidity");
    Serial.print  ("Sensor:       "); Serial.println(sensor.name);
    Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
    Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
    Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println("%");
    Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println("%");
    Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println("%");
    Serial.println("------------------------------------");

    delayMs = sensor.min_delay / 1000;

    // connectivity
    Serial.println(storage::ssid);
    Serial.println(storage::ssid_pass);
    WiFi.begin(storage::ssid, storage::ssid_pass);
    Serial.printf("Connecting to %s", storage::ssid);
    int count = 0;
    bool shouldClose = false;
    while(WiFi.status() != WL_CONNECTED){
        Serial.print(".");
        delay(500);

        count += 500;
        if(count >= TIMEOUT){
            Serial.println(" Failed to connect\nPlease set correct credentials manually or Post data to 192.168.0.2 (make sure you connect to esp hotspot)\nOpening Server... (please connect to ESP_01 wifi ssid)");
            conn::openServer(shouldClose);
            while (!shouldClose){
                conn::server.handleClient();
            }
            Serial.println("Closing Server...");
            count = 0;
            shouldClose = false;
            conn::closeServer();
            Serial.printf("Connecting to %s, %s\n", storage::ssid, storage::ssid_pass);

            WiFi.begin(storage::ssid, storage::ssid_pass);
        }
    }
    Serial.println();

    // websocket
    ws::connect(storage::hostname, " ");
}

void loop() {
    ws::client.loop();  
    while(!ws::isConnected){        // timeout if not on this loop, due to delayMs
        ws::client.loop();
        delay(100);
    }

    delay(delayMs);

    int temp = 8888;
    int hum = 8888;
    sensors_event_t event;

    dht11.temperature().getEvent(&event);
    if(!isnan(event.temperature)) temp = event.temperature;             // C

    dht11.humidity().getEvent(&event);
    if(!isnan(event.relative_humidity)) hum = event.relative_humidity;  // %

    String data = "{ \"suhu\": " + String(temp) + String(',') +
            "\"hum\": " + String(hum) + " }";

    Serial.printf("[%i] %s \n", event.timestamp, data.c_str());

    ws::client.sendTXT(data);
}
