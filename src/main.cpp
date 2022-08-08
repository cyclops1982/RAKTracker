#include <Arduino.h>
#include <SPI.h>
#include <stdio.h>
#include <Wire.h>
#include <vl53l0x_class.h>
#include <SparkFun_Ublox_Arduino_Library.h>
#include "batteryhelper.h"
#include "lorahelper.h"
#include "ledhelper.h"

#ifdef RAK11310
#include "mbed.h"
#include "rtos.h"
#endif

//#define MAX_SAVE
#define SLEEPTIME (1000 * 300)

SemaphoreHandle_t g_taskEvent = NULL;
SoftwareTimer g_taskWakeupTimer;
VL53L0X g_vl53l0x(&Wire, WB_IO2);
SFE_UBLOX_GPS g_GPS;
uint16_t g_msgcount = 0;

void periodicWakeup(TimerHandle_t unused)
{
  // Give the semaphore, so the loop task will wake up
  xSemaphoreGiveFromISR(g_taskEvent, pdFALSE);
}

uint16_t getDistance(void)
{
  g_vl53l0x.begin();
  if (g_vl53l0x.InitSensor(0x52) != VL53L0X_ERROR_NONE)
  {
#ifndef MAX_SAVE
    Serial.println("Init g_vl53l0x failed...");
#endif
    LedHelper::BlinkHalt();
  }

  uint32_t distance;

  if (g_vl53l0x.GetDistance(&distance) != VL53L0X_ERROR_NONE)
  {
    distance = 0;
  }
  g_vl53l0x.VL53L0X_Off();
  g_vl53l0x.end();
  return distance;
}

void setup()
{
  delay(3000); // For whatever reason, some pins/things are not available at startup right away. So we wait 3 seconds for stuff to warm up or something
  LedHelper::init();

  // Initialize serial for output.
#ifndef MAX_SAVE
  time_t timeout = millis();
  Serial.begin(115200);
  // check if serial has become available and if not, just wait for it.
  while (!Serial)
  {
    if ((millis() - timeout) < 5000)
    {
      delay(100);
    }
    else
    {
      break;
    }
  }

  delay(3000); // just wait 3 sec so we can plugin the connection or setup the serial session.
#endif

  // Setup/start the wire that we use for the Sensor.
  Wire.begin();

  // Setup GPS pins
  pinMode(WB_IO2, OUTPUT);
  digitalWrite(WB_IO2, LOW);
  delay(1000);
  digitalWrite(WB_IO2, HIGH);
  delay(1000);

  if (g_GPS.begin() == false)
  {
#ifndef MAX_SAVE
    Serial.println("Ublox GPS not detected at default I2C address. Please check wiring. Halting.");
#endif
    LedHelper::BlinkHalt();
  }

  g_GPS.setDynamicModel(DYN_MODEL_WRIST);
  g_GPS.setI2COutput(COM_TYPE_UBX);

  Serial.println(F("Waiting for a 3D fix..."));

  byte gpsFixType = 0;

  while (gpsFixType < 3)
  {
    gpsFixType = g_GPS.getFixType(); // Get the fix type
    Serial.print(F("Fix: "));        // Print it
    Serial.print(gpsFixType);
    if (gpsFixType == 0)
      Serial.print(F(" = No fix"));
    else if (gpsFixType == 1)
      Serial.print(F(" = Dead reckoning"));
    else if (gpsFixType == 2)
      Serial.print(F(" = 2D"));
    else if (gpsFixType == 3)
      Serial.print(F(" = 3D"));
    else if (gpsFixType == 4)
      Serial.print(F(" = GNSS + Dead reckoning"));
    else if (gpsFixType == 5)
      Serial.print(F(" = Time only"));
    Serial.println();
    LedHelper::BlinkDelay(LED_BLUE, 500);
  }

  LoraHelper::InitAndJoin();

  g_GPS.powerOff(SLEEPTIME);
  g_taskEvent = xSemaphoreCreateBinary();
  xSemaphoreGive(g_taskEvent);
  g_taskWakeupTimer.begin(SLEEPTIME, periodicWakeup);
  g_taskWakeupTimer.start();
}

void loop()
{
  if (xSemaphoreTake(g_taskEvent, portMAX_DELAY) == pdTRUE)
  {
    digitalWrite(LED_GREEN, HIGH); // indicate we're doing stuff

    uint32_t gpsStart = millis();
    byte gpsFixType = 0;
    while (gpsFixType < 3 && (millis() - gpsStart < (1000 * 90)))
    {
      gpsFixType = g_GPS.getFixType(); // Get the fix type
#ifndef MAX_SAVE
      Serial.print(F("Fix: ")); // Print it
      Serial.print(gpsFixType);
      if (gpsFixType == 0)
        Serial.print(F(" = No fix"));
      else if (gpsFixType == 1)
        Serial.print(F(" = Dead reckoning"));
      else if (gpsFixType == 2)
        Serial.print(F(" = 2D"));
      else if (gpsFixType == 3)
        Serial.print(F(" = 3D"));
      else if (gpsFixType == 4)
        Serial.print(F(" = GNSS + Dead reckoning"));
      else if (gpsFixType == 5)
        Serial.print(F(" = Time only"));
      Serial.println();
#endif
      LedHelper::BlinkDelay(LED_BLUE, 500);
    }
    uint16_t gpsTime = millis() - gpsStart;
    uint32_t gpsLat = g_GPS.getLatitude();
    uint32_t gpsLong = g_GPS.getLongitude();
    uint8_t gpsSats = g_GPS.getSIV();

    g_GPS.powerOff(SLEEPTIME);
#ifndef MAX_SAVE
    Serial.printf("GPS details: GPStime: %dms; SATS: %d; FIXTYPE: %d; LAT: %d; LONG: %d;\r\n", gpsTime, gpsSats, gpsFixType, gpsLat, gpsLong);
#endif

    // Create the lora message
    memset(m_lora_app_data.buffer, 0, LORAWAN_APP_DATA_BUFF_SIZE);
    int size = 0;
    m_lora_app_data.port = 2;
    m_lora_app_data.buffer[size++] = 0x02; // device
    m_lora_app_data.buffer[size++] = 0x03; // msg version

    // bat voltage
    uint16_t vbat_mv = BatteryHelper::readVBAT();
    m_lora_app_data.buffer[size++] = vbat_mv >> 8;
    m_lora_app_data.buffer[size++] = vbat_mv;

    // distance
    uint16_t distance = getDistance();
    m_lora_app_data.buffer[size++] = distance >> 8;
    m_lora_app_data.buffer[size++] = distance;

    m_lora_app_data.buffer[size++] = g_msgcount >> 8;
    m_lora_app_data.buffer[size++] = g_msgcount;

    m_lora_app_data.buffer[size++] = gpsTime >> 8;
    m_lora_app_data.buffer[size++] = gpsTime;

    m_lora_app_data.buffer[size++] = gpsSats;
    m_lora_app_data.buffer[size++] = gpsFixType;

    m_lora_app_data.buffer[size++] = gpsLat >> 24;
    m_lora_app_data.buffer[size++] = gpsLat >> 16;
    m_lora_app_data.buffer[size++] = gpsLat >> 8;
    m_lora_app_data.buffer[size++] = gpsLat;

    m_lora_app_data.buffer[size++] = gpsLong >> 24;
    m_lora_app_data.buffer[size++] = gpsLong >> 16;
    m_lora_app_data.buffer[size++] = gpsLong >> 8;
    m_lora_app_data.buffer[size++] = gpsLong;

    m_lora_app_data.buffsize = size;

    lmh_error_status error = lmh_send_blocking(&m_lora_app_data, LMH_CONFIRMED_MSG, 5000);
#ifndef MAX_SAVE
    if (error == LMH_SUCCESS)
    {
      Serial.println("lmh_send ok");
    }
    else
    {
      Serial.printf("lmh_send failed: %d\n", error);
    }
#endif
    g_msgcount++;
    digitalWrite(LED_GREEN, LOW);
  }
  xSemaphoreTake(g_taskEvent, 10);
}
