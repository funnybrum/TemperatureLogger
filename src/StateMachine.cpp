#include "Monitor.h"

bool should_push() {
    if (strlen(settings.getSettings()->influxDB.address)<2) {
        return false;
    }

    RTCSettingsData* data = settings.getRTCSettings();

    uint32_t random = 0;
    uint8_t *randomSeed = (uint8_t*) data;
    for (uint16_t i = 0; i < sizeof(RTCSettingsData); i++) {
        random += *(((uint8_t*)data) + i);
    }
    random %= 5;

    if (data->lastErrorIndex > 0 && data->index - data->lastErrorIndex < 3 + random) {
        // There was an error less than 3 to 8 samples ago. Do not try to push the data.
        return false;
    }

    if (data->index >= 10) {
        // There are 10 or more samples
        return true;
    }

    if (abs(data->lastPushedTemp - data->temp[data->index-1]) > 10) {
        // There is >= 1 degree difference from the last pushed temperature
        return true;
    }

    return false;
}

void read_sensor() {
    bool sensorInit = bme280.begin();
    // Give some time to the sensor so it can take a reading
    delay(10);
    bool sensorRead = bme280.measure();

    RTCSettingsData* data = settings.getRTCSettings();

    if (!sensorInit || !sensorRead) {
        data->sensorErrors++;
    }

    uint8_t maxIndex = sizeof(data->temp)/sizeof(data->temp[0]) - 1;
    if (data->index > maxIndex) {
        for (int i = 0; i < maxIndex; i++) {
            data->temp[i] = data->temp[i+1];
            data->humidity[i] = data->humidity[i+1];
        }
        data->index = maxIndex;
    }

    data->temp[data->index] = round(bme280.getTemperature() * 10);
    data->humidity[data->index] = round(bme280.getHumidity() * 10);

    if (data->lastPushedTemp == 0) {
        data->lastPushedTemp = data->temp[data->index];
    }

    data->index++;
}

void push_data() {
    RTCSettingsData* data = settings.getRTCSettings();
    uint8_t samples = data->index;

    // Note: Telemetry buffer capacity is 16kb. Estimatet values for 90 points pushed at once is ~14kb at most.

    for (uint8_t i = 0; i < samples; i++) {
        uint32_t nowMinusSeconds = data->cycleCompensation + (samples-i-1) * SAMPLING_INTERVAL_NS;
        if (data->state == PUSH) {
            // In deep sleep cycle state, compoensate the micros.
            nowMinusSeconds+=micros();
        }
        nowMinusSeconds = nowMinusSeconds / 1000000;
        float temp = data->temp[i]/10.0f;
        float humidity =  data->humidity[i]/10.0f;
        float abs_humidity = bme280.calculateAbsoluteHumidity(data->humidity[i]/10.0f, data->temp[i]/10.0f);
        dataSender.append("temp", temp, nowMinusSeconds, 1);
        dataSender.append("humidity", humidity, nowMinusSeconds, 1);
        dataSender.append("abs_humidity", abs_humidity, nowMinusSeconds, 2);
    }

    dataSender.append("v_bat", battery.getVoltage()/100.0f, 0, 2);
    dataSender.append("sensor_errors", data->sensorErrors, 0);
    dataSender.append("connect_errors", data->connectErrors, 0);
    dataSender.append("push_errors", data->pushErrors, 0);
    dataSender.append("points", data->index, 0);

    if (dataSender.push()) {
        data->lastPushedTemp = data->temp[samples-1];
        data->index = 0;
        data->lastErrorIndex = 0;
    } else {
        data->pushErrors++;
        data->lastErrorIndex = data->index;
    }
}

void collect_step() {
    read_sensor();

    if (should_push()) {
        // Execute push as next step
        RTCSettingsData* data = settings.getRTCSettings();
        data->state = PUSH;
        data->cycleCompensation = micros() + 1000;
        settings.loop();
        ESP.deepSleep(1000, WAKE_RF_DEFAULT);
    }

    settings.loop();

    // 10ms less sleep needed based on log analysis
    // The min(max(...)) thing is to avoid unexpected case where the awake interval was longer than
    // sampling interval and this would result in a sleep of ~70 minutes.
    ESP.deepSleep(
        min(max(1000UL, SAMPLING_INTERVAL_NS - micros() - 10000), SAMPLING_INTERVAL_NS),
        WAKE_RF_DISABLED);
}

void push_step() {
    wifi.begin();
    wifi.connect();
    while (!wifi.isConnected() && !wifi.isInAPMode() && millis() < 20000) {
        yield();
        delay(10);
        wifi.loop();
    }

    if (wifi.isConnected()) {
        dataSender.init();
        push_data();
    } else {
        settings.getRTCSettings()->connectErrors++;
        settings.getRTCSettings()->lastErrorIndex = settings.getRTCSettings()->index;
        settings.getRTCSettings()->network.wifi_channel = 0;
    }

    wifi.disconnect();
    settings.getRTCSettings()->state = COLLECTING;
    settings.loop();

    // 1780ms more sleep needed based on log analysis
    // The min(max(...)) thing is to avoid unexpected case where the awake interval was longer than
    // sampling interval and this would result in a sleep of ~70 minutes.
    ESP.deepSleep(
        min(max(1000UL, SAMPLING_INTERVAL_NS - micros() - settings.getRTCSettings()->cycleCompensation + 1780000), SAMPLING_INTERVAL_NS),
        WAKE_RF_DISABLED);
}