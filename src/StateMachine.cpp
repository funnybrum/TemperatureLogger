#include "Monitor.h"

void collect() {
#ifdef DEBUG
    Serial.println("Collecting");
#endif

    bme280.begin();
    // Give some time to the sensor so it can take a reading
    delay(10);
    bme280.measure();

    uint8_t index = settings.getRTCSettings()->index;
    settings.getRTCSettings()->temp[index] = round(bme280.getTemperature() * 10);
    settings.getRTCSettings()->humidity[index] = round(bme280.getHumidity() * 10);
    settings.getRTCSettings()->voltage[index] = round(475.0 * analogRead(A0) / 1023);
    settings.getRTCSettings()->index++;


    if (settings.getRTCSettings()->index > 10) {
        // Execute push as next step
        settings.getRTCSettings()->state = PUSH;
        settings.getRTCSettings()->cycleCompensation = micros() + 1000;
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

void push() {
    wifi.begin();
    wifi.connect();
    while (!wifi.isConnected() && !wifi.isInAPMode()) {
        yield();
        delay(1);
        wifi.loop();
    }

#ifdef DEBUG
    Serial.printf("Pushing %d readings\n", settings.getRTCSettings()->index);
    for (uint8_t i=0; i<settings.getRTCSettings()->index; i++) {
        Serial.printf("idx: %d - \t%2.1fC\t%2.1f\n",
                        i,
                        settings.getRTCSettings()->temp[i]/10.0f,
                        settings.getRTCSettings()->humidity[i]/10.0f);
    }
#endif

    dataSender.init();
    uint8_t samples = settings.getRTCSettings()->index;
    for (uint8_t i = 0; i < samples; i++) {
        uint32_t nowMinusSeconds = millis() + settings.getRTCSettings()->cycleCompensation + (samples-i) * SAMPLING_INTERVAL_NS;
        nowMinusSeconds = nowMinusSeconds / 1000000;
        dataSender.append("temp", settings.getRTCSettings()->temp[i]/10.0f, nowMinusSeconds, 1);
        dataSender.append("humidity", settings.getRTCSettings()->humidity[i]/10.0f, nowMinusSeconds, 1);
        dataSender.append("bat_voltage", settings.getRTCSettings()->voltage[i]/100.0f, nowMinusSeconds, 2);
    }
    dataSender.push();

    wifi.disconnect();

    settings.getRTCSettings()->index = 0;
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