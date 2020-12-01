#include "Settings.h"
#include "Monitor.h"

Settings::Settings()
    :SettingsBase(&logger) {
}

void Settings::initializeSettings() {
    strcpy(settingsData.network.hostname, HOSTNAME);
    settingsData.bme280.humidityFactor = 100;
    settingsData.bme280.humidityOffset = 0;
    Serial.print("Settings network: ");
    Serial.println(settingsData.network.ssid);
    Serial.print("Settings channel: ");
    Serial.println(settingsData.network.wifi_channel);
}

SettingsData* Settings::getSettings() {
    return &settingsData;
}