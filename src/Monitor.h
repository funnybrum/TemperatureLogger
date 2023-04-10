#pragma once

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266mDNS.h>

#include "user_interface.h"

#include "Settings.h"
#include "SystemCheck.h"
#include "WiFi.h"
#include "WebServer.h"
#include "Logger.h"
#include "InfluxDBSender.h"


extern Logger logger;
extern Settings settings;
extern WiFiManager wifi;
extern InfluxDBSender dataSender;
extern TempSensor tempSensor;
extern Battery battery;

// If DEBUG is defined, the serial output will output the log messages.
// #define DEBUG

#include "StateMachine.h"
#include "TempSensor.h"
#include "Battery.h"

#define HTTP_PORT 80
#define HOSTNAME "temp-monitor"
#define SAMPLING_INTERVAL_MS 60 * 1000UL   // Interval on which to take measurments in milliseconds
#define MAX_FRESH_BOOT_STATE_DURATION_S 600 // Max time to stay in fresh boot (WiFi on) state
