#include "Settings.h"
#include "Monitor.h"

Settings::Settings()
    :SettingsBase(&logger) {
}

void Settings::initializeSettings() {
    strcpy(settingsData.network.hostname, HOSTNAME);
    settingsData.tempSensorSettings.temperatureFactor = 1000;
    settingsData.tempSensorSettings.temperatureOffset = 0;
    settingsData.tempSensorSettings.humidityFactor = 1000;
    settingsData.tempSensorSettings.humidityOffset = 0;
    settingsData.battery.voltageFactor = 1000;
    settingsData.battery.voltageThreshold = 3000;
}

SettingsData* Settings::getSettings() {
    return &settingsData;
}

RTCSettingsData* Settings::getRTCSettings() {
    return &rtcSettingsData;
}