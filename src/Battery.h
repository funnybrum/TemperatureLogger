#pragma once

struct BatterySettings {
    uint16_t voltageFactor;     // In 0.001 units, i.e. 1000 here sets the factor to 1
    uint16_t voltageThreshold;  // In mV, shut down if battery voltage is below that level
};

const char BATTERY_CONFIG_PAGE[] PROGMEM = R"=====(
<fieldset style='display: inline-block; width: 300px'>
<legend>Battery settings</legend>
Voltage factor:<br>
<input type="text" name="voltage_factor" value="%d"><br>
<small><em>corrected voltage = 0.001 * factor * raw</em></small><br>
Battery protection:<br>
<input type="text" name="voltage_threshold" value="%d"><br>
<small><em>in mV, below that voltage the logger will shut down</em></small><br>
</fieldset>
)=====";

class Battery {
    public:
        Battery(BatterySettings* settings);
        float getVoltage();
        void checkLevel();

        void get_config_page(char* buffer);
        void parse_config_params(WebServerBase* webServer);
    private:
        BatterySettings* _settings;
};
