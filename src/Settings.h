#pragma once

#include "SettingsBase.h"
#include "WiFi.h"
#include "InfluxDBSender.h"
#include "BME280.h"
#include "StateMachine.h"

struct SettingsData {
    NetworkSettings network;
    InfluxDBSenderSettings influxDB;
    BME280Settings bme280;
    struct AQSensor {
        int16_t temperatureOffset;
        int16_t humidityOffset;
    } aqSensor;
    struct HumidMatic {
        uint8_t targetHumidityLow;
        uint8_t targetHumidityHigh;
    } hm;
};

struct RTCSettingsData {
    RTCNetworkSettings network;

    // Temperature and humidity readings buffer
	uint8_t index;
	int16_t temp[30];
	int16_t humidity[30];

    State state;
    uint32_t cycleCompensation;

    uint32_t a;   // A padding. For some reason it doesn't work without it - write operations are failing.
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
