
#ifndef MAIN_H
#define MAIN_H
#include <Arduino.h>
#include <SPI.h>

extern SemaphoreHandle_t g_taskEvent;
extern uint8_t g_EventType;
extern uint8_t g_rcvdLoRaData[];
extern uint8_t g_rcvdDataLen;

#endif