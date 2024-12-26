#ifndef SERIALHELPER_H
#define SERIALHELPER_H

//#define MAX_SAVE

#ifndef MAX_SAVE
#define SERIAL_LOG(fmt, args...)    \
    {                               \
        Serial.printf(fmt, ##args); \
        Serial.println();           \
        Serial.flush();             \
    }
#else
#define SERIAL_LOG(fmt, args...) ;
#endif

#endif