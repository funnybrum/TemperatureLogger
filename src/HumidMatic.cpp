#include "HumidMatic.h"

Logger logger = Logger();
Settings settings = Settings();

WiFiManager wifi = WiFiManager(&logger, &settings.getSettings()->network);
WebServer webServer = WebServer(&logger, &settings.getSettings()->network);

DataCollector dataCollector = DataCollector();
BoschBME280 tempSensor = BoschBME280();

void setup() { 
    Serial.begin(9600);
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

    Serial.print("Temp: ");
    Serial.print(tempSensor.getTemperature());
    Serial.print("C, humidity: ");
    Serial.print(tempSensor.getHumidity());
    Serial.println("%");
    
    delay(1000);
}
