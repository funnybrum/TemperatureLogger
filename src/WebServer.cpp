#include "Monitor.h"

char buffer[4096];

WebServer::WebServer(Logger* logger, NetworkSettings* networkSettings)
    :WebServerBase(networkSettings, logger, NULL) {
}

void WebServer::registerHandlers() {
    server->on("/", std::bind(&WebServer::handle_root, this));
    server->on("/settings", std::bind(&WebServer::handle_settings, this));
    server->on("/get", std::bind(&WebServer::handle_get, this));
    server->on("/stats", std::bind(&WebServer::handle_stats, this));
}

void WebServer::handle_root() {
    server->sendHeader("Location","/settings");
    server->send(303);
}

void WebServer::handle_settings() {
    // Parse passed parameters
    wifi.parse_config_params(this);
    bme280.parse_config_params(this);
    dataSender.parse_config_params(this);

    // Generate settings page content
    char network_settings[strlen_P(NETWORK_CONFIG_PAGE) + 32];
    wifi.get_config_page(network_settings);

    char influx_db_sender_settings[strlen_P(INFLUXDB_SENDER_CONFIG_PAGE) + 64];
    dataSender.get_config_page(influx_db_sender_settings);

    char temp_sensor_settings[strlen_P(BME280_CONFIG_PAGE) + 16];
    bme280.get_config_page(temp_sensor_settings);


    sprintf_P(
        buffer,
        CONFIG_PAGE,
        network_settings,
        influx_db_sender_settings,
        temp_sensor_settings);
    server->send(200, "text/html", buffer);
}

void WebServer::handle_get() {
    sprintf_P(buffer,
              GET_JSON,
              bme280.getTemperature(),
              bme280.getHumidity());
    server->send(200, "application/json", buffer);
}

void WebServer::handle_stats() {
    uint32_t a = 0;
    ESP.rtcUserMemoryWrite(0, &a, 4);
    sprintf_P(buffer,
              PSTR("Uptime: %uds. Free heap: %u"),
              millis()/1000,
              ESP.getFreeHeap());
    server->send(200, "text/plain", buffer);
}
