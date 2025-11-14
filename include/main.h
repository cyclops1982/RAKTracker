
#ifndef MAIN_H
#define MAIN_H
#include <Arduino.h>
#include <SPI.h>
#include <iostream>

enum class EventType
{
  None = 0 << 0,
  Timer = 1 << 1,
  LoraDataReceived = 1 << 2,
  Motion1stInterrupt = 1 << 3,
  Motion2ndInterrupt = 1 << 4
};

inline EventType operator|(EventType a, EventType b)
{
  return static_cast<EventType>(static_cast<int>(a) | static_cast<int>(b));
}

inline EventType operator&(EventType a, EventType b)
{
  return static_cast<EventType>(static_cast<int>(a) & static_cast<int>(b));
}

inline EventType operator~(EventType a)
{
  return static_cast<EventType>(~static_cast<int>(a));
}



inline EventType& operator|=(EventType& a, EventType b) {
    return a = a | b;
}

inline EventType& operator&=(EventType& a, EventType b) {
    return a = a & b;
}

extern SemaphoreHandle_t g_semaphore;
extern EventType g_EventType;
extern BaseType_t g_taskHighPrio;
extern uint8_t g_rcvdLoRaData[];
extern uint8_t g_rcvdDataLen;
extern bool g_lorawan_joined;

void initMotionSensor();

#endif