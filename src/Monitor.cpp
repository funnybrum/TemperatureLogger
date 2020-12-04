#include "Monitor.h"

Logger logger = Logger(false);
Settings settings = Settings();

WiFiManager wifi = WiFiManager(&logger, &settings.getSettings()->network, &settings.getRTCSettings()->network);
WebServer webServer = WebServer(&logger, &settings.getSettings()->network);
BME280 bme280 = BME280();

void setup() { 

#ifdef DEBUG
    Serial.begin(74880);
    while (!Serial) {
        delay(5); // wait for serial port to connect. Needed for native USB
    }
#endif
 
    pinMode(D0, WAKEUP_PULLUP);

    settings.begin();
    logger.begin();

#ifdef DEBUG
    Serial.printf("state: %d, index: %d\n", settings.getRTCSettings()->state, settings.getRTCSettings()->index);
#endif

    switch(settings.getRTCSettings()->state) {
        case FRESH_BOOT:
            wifi.begin();
            webServer.begin();
            wifi.connect();
            while (!wifi.isConnected() && !wifi.isInAPMode()) {
                yield();
                delay(1);
                wifi.loop();
            }
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
    wifi.loop();
    webServer.loop();

    if (millis() > 10000) {
        settings.getRTCSettings()->state = COLLECTING;
        settings.loop();

        Serial.println("Going in deep sleep");
        Serial.flush();
        delay(100);
        ESP.deepSleep(2000000L, WAKE_RF_DISABLED);
    }

    delay(100);
    Serial.print(".");
}
