#include "Monitor.h"
#include "Battery.h"

Battery::Battery(BatterySettings* settings) {
    _settings = settings;
}

float Battery::getVoltage() {
    float voltage = round(475.0 * analogRead(A0) / 1023);
    voltage = voltage * settings.getSettings()->battery.voltageFactor / 1000.0;
    return voltage;   
}

void Battery::get_config_page(char* buffer) {
    sprintf_P(
        buffer,
        BATTERY_CONFIG_PAGE,
        _settings->voltageFactor);
}

void Battery::parse_config_params(WebServerBase* webServer) {
    webServer->process_setting("voltage_offset", _settings->voltageFactor);
}