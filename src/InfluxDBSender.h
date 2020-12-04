#pragma once

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include <ESP8266HTTPClient.h>

#include "Logger.h"
#include "WiFi.h"
#include "WebServerBase.h"

#ifndef TELEMETRY_BUFFER_SIZE
#define TELEMETRY_BUFFER_SIZE 8192
#endif
const char INFLUXDB_SENDER_CONFIG_PAGE[] PROGMEM = R"=====(
<fieldset style='display: inline-block; width: 300px'>
<legend>InfluxDB settings</legend>
InfluxDB integration:<br>
Address:<br>
<input type="text" name="ifx_address" value="%s"><br>
<small><em>like 'http://192.168.0.1:8086'</em></small><br><br>
Database:<br>
<input type="text" name="ifx_db" value="%s"><br>
<small><em>Database for sending the data</em></small><br><br>
</fieldset>
)=====";

struct InfluxDBSenderSettings {
    char address[64];
    char database[16];
};

class InfluxDBSender {
    public:
        InfluxDBSender(Logger* _logger,
                          InfluxDBSenderSettings* settings,
                          NetworkSettings* networkSettings) {
            _logger = _logger;
            _settings = settings;
            _networkSettings = networkSettings;
        }

        void init() {
            _http = new HTTPClient();
            const char * headerKeys[] = {"date"};
            _http->collectHeaders(headerKeys, 1);
            ping();
        }

        void append(const char* metric, float value, uint32_t nowMinusSeconds, uint8_t precision=0) {
            char format[32];
            int metricSize = 0;

            sprintf(format, "%%s,src=%%s value=%%.%df %%ld\n", precision);
            metricSize = snprintf(
                telemetryData + telemetryDataSize,
                TELEMETRY_BUFFER_SIZE - telemetryDataSize,
                format,
                metric,
                _networkSettings->hostname,
                value,
                getTimestamp() - nowMinusSeconds);

            if (telemetryDataSize + metricSize > TELEMETRY_BUFFER_SIZE) {
                // In that case - we don't have the whole metric line in the buffer.
                telemetryData[telemetryDataSize] = '\0';
                _logger->log("Telemetry buffer overflow, data point dropped!");
            } else {
                telemetryDataSize += metricSize;
            }
        }

        void get_config_page(char* buffer) {
            sprintf_P(
                buffer,
                INFLUXDB_SENDER_CONFIG_PAGE,
                _settings->address,
                _settings->database);
        }

        void parse_config_params(WebServerBase* webServer) {
            webServer->process_setting("ifx_address", _settings->address, sizeof(_settings->address));
            webServer->process_setting("ifx_db", _settings->database, sizeof(_settings->database));
        }

    private:
        // Sync the local timestamp based on the date/time response from the InfluxDB server. This
        // is needed in order to append the proper timestamps to the metrics beeing generated.
        void syncTime(const char* dateTime) {
            if (strlen(dateTime) != 29) {
                // Something's wrong. The datetime should be 29 characters.
                _logger->log("Failed to parse the InfluxDB date/time: %s", dateTime);
                return;
            }
            // Get the millis when the remote date was received.
            remoteTimestampMillis = millis();

            // Calculate the timestamp from a date/time string as "Sat, 08 Dec 2018 07:38:17 GMT". Based on
            // the "Seconds Since the Epoch" formula as defined by POSIX:2008 section 4.15. The following
            // are the required parameters:
            int16_t tm_sec = atoi(dateTime+23);         // seconds
            int16_t tm_min = atoi(dateTime+20);         // minutes
            int16_t tm_hour = atoi(dateTime+17);        // hours
            int16_t tm_year = atoi(dateTime+12) - 1900; // calendar year minus 1900
            int16_t tm_yday;                            // passed days since January 1 of the current year

            // tm_yday is calculated based on the month, the day and on the year (leap/non-leap).

            // Day in current month
            int16_t day = atoi(dateTime+5);

            // Convert month to number. 1 for January, 12 for December.
            const char* months[12] = {
                "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
            };
            int16_t month = 1;
            while (strncmp(dateTime + 8, months[month-1], 3) != 0 && month < 13) {
                month++;
            }

            // Determine if the year is leap or not.
            bool isLeapYear = false;
            if ((tm_year + 1900) % 4 == 0 )
                isLeapYear = true;
            if ((tm_year + 1900) % 100 == 0 )
                isLeapYear = false;
            if ((tm_year + 1900) % 400 == 100 )
                isLeapYear = true;

            // Calculate the number of passed days before the current month.
            int16_t days_before_month[2][13] = {
                /* Normal years.  */
                {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365},
                /* Leap years.  */
                {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366}
            };

            // Calculate the full days that have passed since the year start.
            tm_yday = days_before_month[isLeapYear][month-1] + day-1;

            remoteTimestamp =
                tm_sec + tm_min*60 +
                tm_hour*3600 +
                tm_yday*86400 +
                (tm_year-70)*31536000 +
                ((tm_year-69)/4)*86400 -
                ((tm_year-1)/100)*86400 +
                ((tm_year+299)/400)*86400;
        }

        unsigned long getTimestamp() {
            // The below would be correct even if the millis() have rolled over. The only
            // requirement for this to work is to have the last timestamp retrieved before not more
            // than the millis rollover period that is ~50 days.
            unsigned long secondsSinceTimestampRetrieval = (millis() - remoteTimestampMillis) / 1000;

            return remoteTimestamp + secondsSinceTimestampRetrieval;
        }

        // Executed with only purpose to get the current timestamp of the IndluxDB.
        void ping() {
            String url = "";
            url += _settings->address;
            url += "/ping";
            _http->begin(url);
            int httpCode = _http->GET();
            if (httpCode == 204) {
                syncTime(_http->header("date").c_str());
            }
            _http->end();
        }

        bool push() {
            String url = "";
            url += _settings->address;
            url += "/write?precision=s&db=";
            url += _settings->database;

            _http->begin(url);
            int statusCode =_http->POST((uint8_t *)telemetryData, telemetryDataSize-1);  // -1 to remove
                        
            bool success = statusCode == 204;                                                                // the last '\n'.
            if (success) {
                telemetryDataSize = 0;
                syncTime(_http->header("date").c_str());
            } else {
                _logger->log("Push failed with HTTP %d", statusCode);
            }

            _http->end();
            return success;
        }

        char telemetryData[TELEMETRY_BUFFER_SIZE];
        unsigned int telemetryDataSize = 0;
        unsigned long remoteTimestamp;
        unsigned long remoteTimestampMillis;
        HTTPClient* _http = NULL;

        Logger* _logger = NULL;
        InfluxDBSenderSettings* _settings = NULL;
        NetworkSettings* _networkSettings = NULL;
};
