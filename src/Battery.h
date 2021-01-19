#pragma once

struct BatterySettings {
    uint16_t voltageFactor;    // In 0.001 units, i.e. 1000 here sets the factor to 1
};

const char BATTERY_CONFIG_PAGE[] PROGMEM = R"=====(
<fieldset style='display: inline-block; width: 300px'>
<legend>Battery settings</legend>
Voltage offset:<br>
<input type="text" name="voltage_offset" value="%d"><br>
<small><em>corrected voltage = 0.001 * factor * raw</em></small><br>
</fieldset>
)=====";

class Battery {
    public:
        Battery(BatterySettings* settings);
        float getVoltage();

        void get_config_page(char* buffer);
        void parse_config_params(WebServerBase* webServer);
    private:
        BatterySettings* _settings;
};
