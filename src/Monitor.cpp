#include "Monitor.h"

Logger logger = Logger();
Settings settings = Settings();

WiFiManager wifi = WiFiManager(&logger, &settings.getSettings()->network);
WebServer webServer = WebServer(&logger, &settings.getSettings()->network);

DataCollector dataCollector = DataCollector();
BME280 tempSensor = BME280();

void setup() { 
    Serial.begin(9600);

    for (int i = 0; i<4; i++) {
        delay(500);
        Serial.print(".");
    }
    Serial.println(".");

    settings.begin();

    wifi.begin();
    wifi.connect();

    webServer.begin();

    tempSensor.begin();
    dataCollector.begin();
}

void loop() {
    wifi.loop();
    webServer.loop();
    settings.loop();

    tempSensor.loop();
    dataCollector.loop();

    char buf[128];
    sprintf(buf, "Temp %d, humidity %d", round(tempSensor.getTemperature()), round(tempSensor.getHumidity()));
    logger.log(buf);

    delay(10);
}
