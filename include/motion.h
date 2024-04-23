
#ifndef MOTION_H
#define MOTION_H
#include <Arduino.h>
#include <SparkFunLIS3DH.h>
#include "serialhelper.h"
#include "ledhelper.h"

#define MOTION_DISABLED

#ifndef MOTION_DISABLED
class MotionHelper {
    public:
        /// @brief This initializes and/or updates the motion sensor. It turns off when both thresholds are 0 (zero). 
        /// @param firstThreshold 
        /// @param secondThreshold 
        /// @param firstDuration 
        /// @param secondDuration 
        static void InitMotionSensor(uint8_t firstThreshold, uint8_t secondThreshold, uint8_t firstDuration, uint8_t secondDuration);
        static uint8_t GetMotionInterupts();
        static bool IsMotionEnabled();

};
#else
class MotionHelper {
    public:
        static void InitMotionSensor(uint8_t firstThreshold, uint8_t secondThreshold, uint8_t firstDuration, uint8_t secondDuration) {};
        static uint8_t GetMotionInterupts() {
            return 0xFF;
        };
        static bool IsMotionEnabled() { return false; };

};

#endif


#endif