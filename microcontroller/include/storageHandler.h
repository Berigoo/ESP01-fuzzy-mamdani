//
// Created by pach on 1/10/24.
//
#pragma once
#ifndef ESP_STORAGEHANDLER_H
#define ESP_STORAGEHANDLER_H

#define SSID_LEN 16
#define PASS_LEN 16
#define HOSTNAME_LEN 33
//
#include <Arduino.h>
#include <ESP_EEPROM.h>


struct storage{
    static char ssid[SSID_LEN];
    static char ssid_pass[PASS_LEN];
    static char hostname[HOSTNAME_LEN];
};

class storageHandler {
public:
    static bool write();
    static void read();
};

#endif //ESP_STORAGEHANDLER_H
