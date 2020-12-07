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
    bme280.begin();
    // Give some time to the sensor so it can take a reading
    delay(10);
    bme280.measure();

    RTCSettingsData* data = settings.getRTCSettings();

    uint8_t index = data->index;
    uint8_t maxIndex = sizeof(data->temp)/sizeof(data->temp[0]) - 1;
    if (index > maxIndex) {
        for (int i = 0; i < maxIndex; i++) {
            data->temp[i] = data->temp[i+1];
            data->humidity[i] = data->humidity[i+1];
            data->voltage[i] = data->voltage[i+1];
        }
        index = maxIndex;
    }

    data->temp[index] = round(bme280.getTemperature() * 10);
    data->humidity[index] = round(bme280.getHumidity() * 10);
    data->voltage[index] = round(436.5 * analogRead(A0) / 1023);
    data->index++;

    if (data->lastPushedTemp == 0) {
        data->lastPushedTemp = data->temp[index];
    }
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
        dataSender.append("bat_voltage", data->voltage[i]/100.0f, nowMinusSeconds, 2);
        Serial.printf("now()-%d: %.1f\n", nowMinusSeconds, data->temp[i]/10.0f);
    }

    dataSender.append("v_bat", GET_V_BAT/100.0f, 0, 2);


    if (dataSender.push()) {
        data->lastPushedTemp = data->temp[samples-1];
        data->index = 0;
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

    // Execute again collect as next step
    settings.loop();
    Serial.println(micros());
    ESP.deepSleep(
        SAMPLING_INTERVAL_NS - micros(),
        WAKE_RF_DISABLED);
}

void push_step() {
    wifi.begin();
    wifi.connect();
    while (!wifi.isConnected() && !wifi.isInAPMode() && micros() < 10000) {
        yield();
        delay(10);
        wifi.loop();
    }
    
    if (wifi.isConnected()) {
        dataSender.init();
        push_data();
    }

    wifi.disconnect();
    settings.getRTCSettings()->state = COLLECTING;
    settings.loop();

    // This may needs to be calibrated a bit. There is likely some delay before the micros start
    // ticking. The drift over the time should be calculated and corrected either here or in the
    // deep sleep performed by the collect function code.
    Serial.println(micros());
    ESP.deepSleep(
        SAMPLING_INTERVAL_NS - micros() - settings.getRTCSettings()->cycleCompensation,
        WAKE_RF_DISABLED);
}