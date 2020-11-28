#include "Settings.h"
#include "Monitor.h"

Settings::Settings()
    :SettingsBase(&logger) {
}

void Settings::initializeSettings() {
    strcpy(settingsData.network.hostname, HOSTNAME);
}

SettingsData* Settings::getSettings() {
    return &settingsData;
}