
#ifndef LEDHELPER_H
#define LEDHELPER_H
#include <Arduino.h>

class LedHelper
{
public:
    static void init();
    static void BlinkHalt(uint blickcount);
    static void BlinkDelay(int ledpin, int delay);
    static void BlinkStatus(uint blickcount);
    static bool isInitialized;
};
#endif