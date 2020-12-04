#pragma once

enum State {
    FRESH_BOOT=0,     // Battery has just been put in place. Boot in config mode - 10 minutes with
                      // no power preserving
    COLLECTING=1,     // Wake up, collect data and either stay in COLLECTING and deep sleep or go
                      // in WIFI_PREPARE
    PUSH=2            // Connect, push, clean buffers and go to COLLECTING
};


void collect();
void push();