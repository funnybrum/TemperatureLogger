#include "Monitor.h"
#include "BME280.h"

BME280::BME280(BME280Settings* settings) {
    _settings = settings;
}

bool BME280::begin() {
    if (!_initialized) {
        _sensorFound = _bme280.begin(0x77);
        if (!_sensorFound) {
            logger.log("BME280 not found on 0x77");    
        }
        if (_settings->humidityFactor == 0) {
            // Initialize settings
            _settings->humidityFactor = 100;
            _settings->humidityOffset = 0;
            _settings->temperatureOffset = 0;
        }
        _initialized = true;
    }

    return _sensorFound;
}

bool BME280::measure() {
    if (!_sensorFound) {
        return false;
    }

    bool ok = _bme280.measure();
    
    if (ok) {
        _temp = _bme280.getTemperature();
        _humidity = _bme280.getHumidity();

        // Apply corrections
        _humidity = _settings->humidityFactor * _humidity * 0.01f +
                    _settings->humidityOffset * 0.1f;

        if (_humidity < 0 || _humidity > 100) {
            logger.log("Incorrect humidity correction, got %f.", _humidity);
            _humidity = max(_humidity, 0.0f);
            _humidity = min(_humidity, 100.0f);
        }

        _temp = _temp + _settings->temperatureOffset / 10.0;
        return true;
    } else {
        logger.log("Failed on BME280 .measure()");
        return false;
    }
}

float BME280::getTemperature() {
    return _temp;
}

float BME280::getHumidity() {
    return _humidity;
}

float BME280::getAbsoluteHimidity() {
    return rh_to_ah(_humidity, _temp);
}

void BME280::get_config_page(char* buffer) {
    sprintf_P(
        buffer,
        BME280_CONFIG_PAGE,
        _settings->temperatureOffset,
        _settings->humidityFactor,
        _settings->humidityOffset);
}

void BME280::parse_config_params(WebServerBase* webServer) {
    webServer->process_setting("temp_offset", _settings->temperatureOffset);
    webServer->process_setting("humidity_factor", _settings->humidityFactor);
    webServer->process_setting("humidity_offset", _settings->humidityOffset);
}

float BME280::rh_to_ah(float rh, float temp) {
    // https://carnotcycle.wordpress.com/2012/08/04/how-to-convert-relative-humidity-to-absolute-humidity/
    double p_sat = 6.112 * pow(EULER, (17.67 * temp) / (temp + 243.5));
    return (p_sat * rh * 2.167428434) / (273.15 + temp);
}
