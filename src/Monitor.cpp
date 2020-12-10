#include "Monitor.h"


#ifdef DEBUG
Logger logger = Logger(true);
#else
Logger logger = Logger(false);
#endif
Settings settings = Settings();

WiFiManager wifi = WiFiManager(&logger, &settings.getSettings()->network, &settings.getRTCSettings()->network);
WebServer webServer = WebServer(&logger, &settings.getSettings()->network);
InfluxDBSender dataSender = InfluxDBSender(&logger, &settings.getSettings()->influxDB, &settings.getSettings()->network);
BME280 bme280 = BME280(&settings.getSettings()->bme280);

void setup() { 

#ifdef DEBUG
    Serial.begin(74880);
    while (!Serial) {
        delay(5); // wait for serial port to connect. Needed for native USB
    }
#endif
 
    pinMode(16, WAKEUP_PULLUP);

    logger.begin();
    settings.begin();
 
    // TODO check if bettery is being charged and if so - go in FRESH_BOOT state.

    if (strlen(settings.getSettings()->network.ssid) < 2) {
        // No configuration. In such case stay in FRESH_BOOT mode.
        settings.getRTCSettings()->state = FRESH_BOOT;
        ESP.eraseConfig();
    }

    switch(settings.getRTCSettings()->state) {
        case FRESH_BOOT:
            wifi.begin();
            webServer.begin();
            bme280.begin();
            wifi.connect();
            while (!wifi.isConnected() && !wifi.isInAPMode()) {
                yield();
                delay(1);
                wifi.loop();
            }
            settings.getRTCSettings()->state = COLLECTING;
            break;
        case COLLECTING:
            collect_step();
            break;
        case PUSH:
            push_step();
            break;
    }
}

uint32_t lastSensorReading = 0;

void loop() {
    settings.loop();
    logger.loop();
    bme280.measure();
    wifi.loop();
    webServer.loop();

    if (lastSensorReading == 0 || millis() - lastSensorReading > SAMPLING_INTERVAL_NS / 1000) {
        lastSensorReading = millis();
        read_sensor();
        if (should_push()) {
            dataSender.init();
            push_data();
        }        
    }

    Serial.print(".");
    delay(1000);
}
