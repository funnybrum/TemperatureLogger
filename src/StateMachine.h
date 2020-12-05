#pragma once

enum State {
    FRESH_BOOT=0,     // Battery has just been put in place. Boot in config mode - 10 minutes with
                      // no power preserving
    COLLECTING=1,     // Wake up, collect data and either stay in COLLECTING and deep sleep or go
                      // in WIFI_PREPARE
    PUSH=2            // Connect, push, clean buffers and go to COLLECTING
};


// Perform the collect step
void collect_step();

// Perform the push step
void push_step();

// Check if the collected data should be pushed
bool should_push();

// Read the BME280 sensor and store its data in the RTC memory structure
void read_sensor();

// Push the collected data in the RTC memory structure to the InfluxDB  
void push_data();
