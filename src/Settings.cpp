#include "Settings.h"
#include "Monitor.h"

Settings::Settings()
    :SettingsBase(&logger) {
}

void Settings::initializeSettings() {
    strcpy(settingsData.network.hostname, HOSTNAME);
    settingsData.bme280.humidityFactor = 100;
    settingsData.bme280.humidityOffset = 0;
    settingsData.battery.voltageFactor = 1000;
}

SettingsData* Settings::getSettings() {
    return &settingsData;
}

RTCSettingsData* Settings::getRTCSettings() {
    return &rtcSettingsData;
}