#ifndef SERIALHELPER_H
#define SERIALHELPER_H

#define MAX_SAVE false
#define SERIAL_LOG(fmt, args...)        \
    {                                   \
        if (MAX_SAVE == false)          \
        {                               \
            Serial.printf(fmt, ##args); \
            Serial.println();           \
        }                               \
    }

#endif