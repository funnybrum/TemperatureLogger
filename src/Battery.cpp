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

void Battery::checkLevel() {
    if (getVoltage() * 1000 < settings.getSettings()->battery.voltageThreshold) {
        ESP.deepSleepInstant(0, WAKE_RF_DISABLED);
    }
}

void Battery::get_config_page(char* buffer) {
    sprintf_P(
        buffer,
        BATTERY_CONFIG_PAGE,
        _settings->voltageFactor,
        _settings->voltageThreshold);
}

void Battery::parse_config_params(WebServerBase* webServer) {
    webServer->process_setting("voltage_factor", _settings->voltageFactor);
    webServer->process_setting("voltage_threshold", _settings->voltageThreshold);
}