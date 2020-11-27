#pragma once

#include "InfluxDBCollector.h"

class DataCollector: public InfluxDBCollector {
    public:
        DataCollector();
        bool shouldCollect();
        void collectData();
        void onPush();
        bool shouldPush();
    private:
        float lastTemp = -1;
        float lastPushedTemp = -1;
        float lastHumidity = -1;
        float lastPushedHumidity = -1;
};
