//
// Created by pach on 1/10/24.
//
#include "../include/conf.h"
#include "../include/storageHandler.h"

char storage::ssid[SSID_LEN] = SSID;
char storage::ssid_pass[PASS_LEN] = SSID_PASS;
char storage::hostname[HOSTNAME_LEN] = HOSTNAME;

bool storageHandler::write() {
    char arr[SSID_LEN + PASS_LEN + HOSTNAME_LEN];

    memcpy(&arr[0], &storage::ssid, SSID_LEN);
    memcpy(&arr[SSID_LEN], &storage::ssid_pass, PASS_LEN);
    memcpy(&arr[SSID_LEN + PASS_LEN], &storage::hostname, HOSTNAME_LEN);

    EEPROM.put(0, arr);

    if(EEPROM.commit()){
        Serial.println("Success to write into EEPROM");
        return true;
    }else {
        Serial.println("Failed to write into EEPROM");
        return false;
    }
}

void storageHandler::read() {
    EEPROM.get(0, storage::ssid);
    EEPROM.get(SSID_LEN, storage::ssid_pass);
    EEPROM.get(SSID_LEN + PASS_LEN, storage::hostname);
}
