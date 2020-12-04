#include "Monitor.h"
#include "BME280.h"

bool BME280::begin() {
    sensorFound = bme280.begin(0x76);
    if (!sensorFound) {
        logger.log("BME280 not found on 0x77");    
    }
    return sensorFound;
}

bool BME280::measure() {
    if (!sensorFound) {
        return false;
    }

    bool ok = bme280.measure();
    
    if (ok) {
        temp = bme280.getTemperature();
        humidity = bme280.getHumidity();

        // Apply corrections
        humidity = settings.getSettings()->bme280.humidityFactor * humidity * 0.01f +
                    settings.getSettings()->bme280.humidityOffset * 0.1f;

        if (humidity < 0 || humidity > 100) {
            logger.log("Incorrect humidity correction, got %f.", humidity);
            humidity = max(humidity, 0.0f);
            humidity = min(humidity, 100.0f);
        }

        temp = temp + settings.getSettings()->bme280.temperatureOffset / 10.0;
        return true;
    } else {
        logger.log("Failed on BME280 .measure()");
        return false;
    }
}

float BME280::getTemperature() {
    return temp;
}

float BME280::getHumidity() {
    return humidity;
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

void BME280::parse_config_params(WebServerBase* webServer) {
    webServer->process_setting("temp_offset", settings.getSettings()->bme280.temperatureOffset);
    webServer->process_setting("pressure_offset", settings.getSettings()->bme280.pressureOffset);
    webServer->process_setting("humidity_factor", settings.getSettings()->bme280.humidityFactor);
    webServer->process_setting("humidity_offset", settings.getSettings()->bme280.humidityOffset);
}

float BME280::rhToAh(float rh, float temp) {
    // https://carnotcycle.wordpress.com/2012/08/04/how-to-convert-relative-humidity-to-absolute-humidity/
    double p_sat = 6.112 * pow(EULER, (17.67 * temp) / (temp + 243.5));
    return (p_sat * rh * 2.167428434) / (273.15 + temp);
}
