#include "Settings.h"
#include "Monitor.h"

Settings::Settings()
    :SettingsBase(&logger) {
}

void Settings::initializeSettings() {
    strcpy(settingsData.network.hostname, HOSTNAME);
    settingsData.bme280.temperatureFactor = 1000;
    settingsData.bme280.temperatureOffset = 0;
    settingsData.bme280.humidityFactor = 1000;
    settingsData.bme280.humidityOffset = 0;
    settingsData.battery.voltageFactor = 1000;
    settingsData.battery.voltageThreshold = 3500;
}

SettingsData* Settings::getSettings() {
    return &settingsData;
}

RTCSettingsData* Settings::getRTCSettings() {
    return &rtcSettingsData;
}