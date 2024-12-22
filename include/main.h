
#ifndef MAIN_H
#define MAIN_H
#include <Arduino.h>
#include <SPI.h>

enum EventType {
  None = -1,
  Timer = 1,
  LoraDataReceived = 2
};

extern SemaphoreHandle_t g_taskEvent;
extern EventType g_EventType;
extern uint8_t g_rcvdLoRaData[];
extern uint8_t g_rcvdDataLen;
extern bool g_lorawan_joined;
extern bool g_lorawan_msgconfirmed;

void initMotionSensor();

#endif