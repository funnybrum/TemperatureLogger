#pragma once

#include "BoschBME280.h"

#define BME280_READ_INTERVAL 1000

struct BME280Settings {
    int8_t temperatureOffset;  // In 0.1C
    int16_t humidityFactor;    // In 0.01 units, i.e. 100 here sets the factor to 1
    int8_t humidityOffset;     // In 0.1%
};

const char BME280_CONFIG_PAGE[] PROGMEM = R"=====(
<fieldset style='display: inline-block; width: 300px'>
<legend>BME280 settings</legend>
Temperature offset:<br>
<input type="text" name="temp_offset" value="%d"><br>
<small><em>in 0.1 degrees, from -125 to 125</em></small><br>
<br>
Humidity correction:<br>
<small>Used for precise sensor calibration<br>corrected = 0.01*factor*raw + 0.1*offset</small><br>
Humidity factor:<br>
<input type="text" name="humidity_factor" value="%d"><br>
Humidity offset:<br>
<input type="text" name="humidity_offset" value="%d"><br>
</fieldset>
)=====";

class BME280 {
    public:
        BME280(BME280Settings* settings);
        bool begin();
        bool measure();

        float getTemperature();
        float getHumidity();
        float getAbsoluteHimidity();

        void get_config_page(char* buffer);
        void parse_config_params(WebServerBase* webServer);

    private:
        float rh_to_ah(float rh, float temp);

        float _temp;
        float _humidity;
        bool _sensorFound;
        BoschBME280 _bme280 = BoschBME280();
        BME280Settings* _settings;
};
