#include "Monitor.h"

Logger logger = Logger(true);
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
 
    pinMode(D0, WAKEUP_PULLUP);

    logger.begin();
    settings.begin();

#ifdef DEBUG
    Serial.printf("state: %d, index: %d\n", settings.getRTCSettings()->state, settings.getRTCSettings()->index);
#endif

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
            collect();
            break;
        case PUSH:
            push();
            break;
    }
}

void loop() {
    settings.loop();
    logger.loop();
    bme280.measure();
    wifi.loop();
    webServer.loop();

    delay(1000);
    Serial.print(".");
}
