#include "ledhelper.h"

LedHelper::LedHelper()
{
    pinMode(LED_BLUE, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    digitalWrite(LED_BLUE, HIGH);
    digitalWrite(LED_GREEN, HIGH);
    delay(500);
    digitalWrite(LED_BLUE, LOW);
    digitalWrite(LED_GREEN, LOW);
}

void LedHelper::BlinkHalt()
{
    while (1)
    {
        digitalWrite(LED_BLUE, HIGH);
        delay(300);
        digitalWrite(LED_BLUE, LOW);
        delay(300);
    }
}
