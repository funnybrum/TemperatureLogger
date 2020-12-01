#pragma once

#include "SettingsBase.h"
#include "WiFi.h"
#include "InfluxDBCollector.h"
#include "BME280.h"

struct SettingsData {
    NetworkSettings network;
    InfluxDBCollectorSettings influxDB;
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
    RtcNetworkSettings network;

	int16_t temp[120];
	int16_t humidity[120];
	uint8_t index;
};

class Settings: public SettingsBase<SettingsData> {
    public:
        Settings();
        SettingsData* getSettings();

    protected:
        void initializeSettings();

    private:
        SettingsData settingsData;
};
