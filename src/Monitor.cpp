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
TempSensor tempSensor = TempSensor(&settings.getSettings()->tempSensorSettings);
Battery battery = Battery(&settings.getSettings()->battery);

void setup() { 

#ifdef DEBUG
    Serial.begin(74880);
    while (!Serial) {
        delay(5); // wait for serial port to connect. Needed for native USB
    }
#endif
 
    logger.begin();
    settings.begin();

    pinMode(16, WAKEUP_PULLUP);

    if (strlen(settings.getSettings()->network.ssid) < 2) {
        // No configuration. In such case stay in FRESH_BOOT mode.
        settings.getRTCSettings()->state = FRESH_BOOT;
        ESP.eraseConfig();
    }

    switch(settings.getRTCSettings()->state) {
        case FRESH_BOOT:
            wifi.begin();
            webServer.begin();
            tempSensor.begin();
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
    tempSensor.measure();
    wifi.loop();
    webServer.loop();

    if (lastSensorReading == 0 || millis() - lastSensorReading > SAMPLING_INTERVAL_MS) {
        lastSensorReading = millis();
        read_sensor();
        if (should_push() || millis() > MAX_FRESH_BOOT_STATE_DURATION_S * 1000) {
            dataSender.init();
            push_data();
            if (millis() > MAX_FRESH_BOOT_STATE_DURATION_S * 1000) {
                settings.loop();
                ESP.deepSleepInstant(SAMPLING_INTERVAL_MS * 1000, WAKE_RF_DISABLED);
            }
        }        
    }

#ifdef DEBUG
    Serial.print(".");
#endif

    delay(1000);
}
