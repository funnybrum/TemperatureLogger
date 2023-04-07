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
	uint8_t index;          // Index of the first unused sample in the buffer.
	int16_t temp[120];       // Temperature buffer. In 0.1C, or 230 = 23C.
	int8_t humidity[120];   // Humidity buffer.

    // Target state of the state machine
    State state;

    // Used on pre-push procedures to compensate for the last cycle duration 
    uint32_t cycleCompensation;

    // Keep track of the last pushed temperature value
    int16_t lastPushedTemp;
    int16_t lastPushedHumidity;

    int16_t pushErrors;
    int16_t connectErrors;
    int16_t sensorErrors;
    int16_t lastErrorIndex;

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
