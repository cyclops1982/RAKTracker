#include "ledhelper.h"

bool LedHelper::isInitialized = false;

void LedHelper::init()
{
    if (!isInitialized)
    {
        pinMode(LED_BLUE, OUTPUT);
        pinMode(LED_GREEN, OUTPUT);
        delay(100);
        digitalWrite(LED_BLUE, LOW);
        digitalWrite(LED_GREEN, LOW);
        // at startup, do a quick blink
        digitalWrite(LED_BLUE, HIGH);
        digitalWrite(LED_GREEN, HIGH);
        delay(500);
        digitalWrite(LED_BLUE, LOW);
        digitalWrite(LED_GREEN, LOW);

        isInitialized = true;
    }
}

void LedHelper::BlinkDelay(int ledpin, int waittime)
{
    init();
    digitalWrite(ledpin, HIGH);
    delay(waittime);
    digitalWrite(ledpin, LOW);
    delay(waittime);
}

void LedHelper::BlinkStatus(uint blickcount)
{
    init();
    digitalWrite(LED_BLUE, LOW);  // make sure it's low.
    for (uint i = 0; i < blickcount; i++)
    {
        digitalWrite(LED_BLUE, HIGH); // High for indicated amount of time.
        delay(500);
        digitalWrite(LED_BLUE, LOW);
        delay(500);
    }
};

void LedHelper::BlinkHalt(uint blickcount)
{
    init();
    while (1)
    {
        for (uint i = 0; i < blickcount; i++)
        {
            digitalWrite(LED_BLUE, HIGH);
            delay(200);
            digitalWrite(LED_BLUE, LOW);
            delay(200);
        }
        delay(2000);
    }
}
