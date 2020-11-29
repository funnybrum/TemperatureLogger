#include "Monitor.h"
#include "BME280.h"

void BME280::begin() {
    sensorFound = bme280.begin(0x76);
    if (!sensorFound) {
        logger.log("BME280 not found on 0x77");    
    }
    this->lastRead = millis() - BME280_READ_INTERVAL + 200;
    if (settings.getSettings()->bme280.humidityFactor == 0) {
        // Set non-zero default values.
        settings.getSettings()->bme280.humidityFactor = 100;
        settings.getSettings()->bme280.humidityOffset = 0;
    }
}

void BME280::loop() {
    if (!sensorFound) {
        return;
    }

    unsigned long timeSinceLastStateUpdate = millis() - this->lastRead;
    if (timeSinceLastStateUpdate > BME280_READ_INTERVAL) {
        lastRead = millis();
        bool ok = bme280.measure();
        if (ok) {
            raw_temp = temp = bme280.getTemperature();
            raw_humidity = humidity = bme280.getHumidity();

            // Apply corrections
            // 1. Apply relative humidity factor and offset. This is for BME280 sensors that still
            // don't provide correct relative humidity after reconditioning.
            humidity = settings.getSettings()->bme280.humidityFactor * humidity * 0.01f +
                       settings.getSettings()->bme280.humidityOffset * 0.1f;

            if (humidity < 0 || humidity > 100) {
                logger.log("Incorrect humidity correction, got %f.", humidity);
                humidity = max(humidity, 0.0f);
                humidity = min(humidity, 100.0f);
            }

            // 2. Get absolute humidity. This will be used to correct relative humidity below
            // after temperature is corrected.
            float ah = getAbsoluteHimidity();

            // 3. Temperature offset. This is not for BME280 issues, but for sensor positioning
            // correction. I.e. if the sensor is near the ceiling - the temperature would be a bit
            // higher.
            temp = temp + settings.getSettings()->bme280.temperatureOffset / 10.0;
        } else {
            logger.log("Failed on BME280 .measure()");
        }
    }
}

float BME280::getTemperature() {
    return temp;
}

float BME280::getHumidity() {
    return humidity;
}

float BME280::getRawTemperature() {
    return raw_temp;
}

float BME280::getRawHumidity() {
    return raw_humidity;
}

float BME280::getPressure() {
    return pressure;
}

float BME280::getAbsoluteHimidity() {
    return rhToAh(this->humidity, this->temp);
}

void BME280::get_config_page(char* buffer) {
    sprintf_P(
        buffer,
        BME280_CONFIG_PAGE,
        settings.getSettings()->bme280.temperatureOffset,
        settings.getSettings()->bme280.pressureOffset,
        settings.getSettings()->bme280.humidityFactor,
        settings.getSettings()->bme280.humidityOffset);
}

void BME280::parse_config_params(WebServerBase* webServer, bool& save) {
    webServer->process_setting("temp_offset", settings.getSettings()->bme280.temperatureOffset, save);
    webServer->process_setting("pressure_offset", settings.getSettings()->bme280.pressureOffset, save);
    webServer->process_setting("humidity_factor", settings.getSettings()->bme280.humidityFactor, save);
    webServer->process_setting("humidity_offset", settings.getSettings()->bme280.humidityOffset, save);
}

float BME280::rhToAh(float rh, float temp) {
    // https://carnotcycle.wordpress.com/2012/08/04/how-to-convert-relative-humidity-to-absolute-humidity/
    double p_sat = 6.112 * pow(EULER, (17.67 * temp) / (temp + 243.5));
    return (p_sat * rh * 2.167428434) / (273.15 + temp);
}
