#include "Monitor.h"

bool should_push() {
    if (strlen(settings.getSettings()->influxDB.address)<2) {
        return false;
    }

    RTCSettingsData* data = settings.getRTCSettings();
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
    for (uint8_t i = 0; i < samples; i++) {
        uint32_t nowMinusSeconds = data->cycleCompensation + (samples-i-1) * SAMPLING_INTERVAL_NS;
        if (data->state == PUSH) {
            // In deep sleep cycle state, compoensate the micros.
            nowMinusSeconds+=micros();
        }
        nowMinusSeconds = nowMinusSeconds / 1000000;
        dataSender.append("temp", data->temp[i]/10.0f, nowMinusSeconds, 1);
        dataSender.append("humidity", data->humidity[i]/10.0f, nowMinusSeconds, 1);
    }

    dataSender.append("v_bat", battery.getVoltage()/100.0f, 0, 2);
    dataSender.append("sensor_errors", data->sensorErrors, 0);
    dataSender.append("connect_errors", data->connectErrors, 0);
    dataSender.append("push_errors", data->pushErrors, 0);
    dataSender.append("points", data->index, 0);

    if (dataSender.push()) {
        data->lastPushedTemp = data->temp[samples-1];
        data->index = 0;
    } else {
        data->pushErrors++;
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

    // 90ms less sleep needed based on log analysis
    // The min(max(...)) thing is to avoid unexpected case where the awake interval was longer than
    // sampling interval and this would result in a sleep of ~70 minutes.
    ESP.deepSleep(
        min(max(1000UL, SAMPLING_INTERVAL_NS - micros() - 90000), SAMPLING_INTERVAL_NS),
        WAKE_RF_DISABLED);
}

void push_step() {
    wifi.begin();
    wifi.connect();
    while (!wifi.isConnected() && !wifi.isInAPMode() && millis() < 10000) {
        yield();
        delay(10);
        wifi.loop();
    }

    if (wifi.isConnected()) {
        dataSender.init();
        push_data();
    } else {
        settings.getRTCSettings()->connectErrors++;
        settings.getRTCSettings()->network.wifi_channel = 0;
    }

    wifi.disconnect();
    settings.getRTCSettings()->state = COLLECTING;
    settings.loop();

    // 390ms more sleep needed based on log analysis
    // The min(max(...)) thing is to avoid unexpected case where the awake interval was longer than
    // sampling interval and this would result in a sleep of ~70 minutes.
    ESP.deepSleep(
        min(max(1000UL, SAMPLING_INTERVAL_NS - micros() - settings.getRTCSettings()->cycleCompensation + 390000), SAMPLING_INTERVAL_NS),
        WAKE_RF_DISABLED);
}