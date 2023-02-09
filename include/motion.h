
#ifndef MOTION_H
#define MOTION_H
#include <Arduino.h>
#include <SparkFunLIS3DH.h>
#include "serialhelper.h"
#include "ledhelper.h"

#define MOTION_ENABLED

#ifdef MOTION_ENABLED

class MotionHelper {
    public:
        static void InitMotionSensor(uint8_t firstThreshold, uint8_t secondThreshold, uint8_t firstDuration, uint8_t secondDuration);
        static uint8_t GetMotionInterupts();

};
#else
class MotionHelper {
    public:
        static void InitMotionSensor(uint8_t firstThreshold, uint8_t secondThreshold, uint8_t firstDuration, uint8_t secondDuration) {};
        static uint8_t GetMotionInterupts() {
            return 0xFF;
        };

};

#endif


#endif