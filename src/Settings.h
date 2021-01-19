#pragma once

#include "SettingsBase.h"
#include "WiFi.h"
#include "InfluxDBSender.h"
#include "BME280.h"
#include "Battery.h"
#include "StateMachine.h"

struct SettingsData {
    NetworkSettings network;
    InfluxDBSenderSettings influxDB;
    BME280Settings bme280;
    BatterySettings battery;
};

struct RTCSettingsData {
    RTCNetworkSettings network;

    // Samples buffer and index for it
	uint8_t index;
	int16_t temp[90];
	int16_t humidity[90];

    // Target state of the state machine
    State state;

    // Used on pre-push procedures to compensate for the last cycle duration 
    uint32_t cycleCompensation;

    // Keep track of the last pushed temperature value
    int16_t lastPushedTemp;

    int16_t pushErrors;
    int16_t connectErrors;
    int16_t sensorErrors;
    int16_t padding;

};

class Settings: public SettingsBase<SettingsData, RTCSettingsData> {
    public:
        Settings();
        SettingsData* getSettings();
        RTCSettingsData* getRTCSettings();

    protected:
        void initializeSettings();

    private:
        SettingsData settingsData;
        RTCSettingsData rtcSettingsData;
};
