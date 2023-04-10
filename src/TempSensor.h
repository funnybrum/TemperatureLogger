#pragma once

#include "BoschBME280.h"

struct TempSensorSettings {
    int16_t temperatureFactor; // In 0.001 units, i.e. 1000 here sets the factor to 1
    int16_t temperatureOffset;  // In 0.01C, i.e. 100 here is 1 degree offset
    int16_t humidityFactor;    // In 0.001 units, i.e. 1000 here sets the factor to 1
    int16_t humidityOffset;     // In 0.01C, i.e. 100 here is 1% RH offset
};

const char TEMP_SENSOR_CONFIG_PAGE[] PROGMEM = R"=====(
<fieldset style='display: inline-block; width: 300px'>
<legend>Temp sensor settings</legend>
<small>Used for precise sensor calibration<br>corrected = 0.001*factor*raw + 0.01*offset</small><br>
Temperature correction:<br>
Temperature factor:<br>
<input type="text" name="temp_factor" value="%d"><br>
Temperature offset:<br>
<input type="text" name="temp_offset" value="%d"><br>
<br>
Humidity correction:<br>
Humidity factor:<br>
<input type="text" name="humidity_factor" value="%d"><br>
Humidity offset:<br>
<input type="text" name="humidity_offset" value="%d"><br>
</fieldset>
)=====";

class TempSensor {
    public:
        TempSensor(TempSensorSettings* settings);
        bool begin();
        bool measure();

        float getTemperature();
        float getHumidity();
        float getAbsoluteHimidity();
        float calculateAbsoluteHumidity(float rh, float temp);

        void get_config_page(char* buffer);
        void parse_config_params(WebServerBase* webServer);

    private:
        float _temp;
        float _humidity;
        bool _sensorFound;
        bool _initialized = false;
        BoschBME280 _bme280 = BoschBME280();
        TempSensorSettings* _settings;
};
