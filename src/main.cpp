#include <Arduino.h>
#include <SPI.h>
#include <stdio.h>
#include <Wire.h>
#include <vl53l0x_class.h>
#include "batteryhelper.h"
#include "lorahelper.h"
#include "ledhelper.h"

#ifdef RAK11310
#include "mbed.h"
#include "rtos.h"
#endif

#define MAX_SAVE

SemaphoreHandle_t taskEvent = NULL;
SoftwareTimer taskWakeupTimer;
VL53L0X sensor_vl53l0x(&Wire, WB_IO2);

void periodicWakeup(TimerHandle_t unused)
{
  // Give the semaphore, so the loop task will wake up
  xSemaphoreGiveFromISR(taskEvent, pdFALSE);
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

  LoraHelper::InitAndJoin();

  taskEvent = xSemaphoreCreateBinary();
  xSemaphoreGive(taskEvent);
  taskWakeupTimer.begin((1000 * 300), periodicWakeup);
  taskWakeupTimer.start();
}

uint16_t msgcount = 0;
void loop()
{
  if (xSemaphoreTake(taskEvent, portMAX_DELAY) == pdTRUE)
  {
    digitalWrite(LED_GREEN, HIGH); // indicate we're doing stuff
    sensor_vl53l0x.begin();

    // Initialize VL53L0X component.
    if (sensor_vl53l0x.InitSensor(0x52) != VL53L0X_ERROR_NONE)
    {
#ifndef MAX_SAVE
      Serial.println("Init sensor_vl53l0x failed...");
#endif
      LedHelper::BlinkHalt();
    }

    uint32_t distance;
    int status;

    status = sensor_vl53l0x.GetDistance(&distance);

    if (status == VL53L0X_ERROR_NONE)
    {
      if (distance < 50)
      {
        digitalWrite(LED_GREEN, HIGH);
      }
      else
      {
        digitalWrite(LED_GREEN, LOW);
      }
    }
    else
    {
      distance = 0;
    }
    sensor_vl53l0x.VL53L0X_Off();
    sensor_vl53l0x.end();
    uint16_t vbat_mv = BatteryHelper::readVBAT();

    memset(m_lora_app_data.buffer, 0, LORAWAN_APP_DATA_BUFF_SIZE);
    int size = 0;
    m_lora_app_data.port = 2;
    m_lora_app_data.buffer[size++] = 0x02; // device
    m_lora_app_data.buffer[size++] = 0x02; // msg version

    // bat voltage
    m_lora_app_data.buffer[size++] = vbat_mv >> 8;
    m_lora_app_data.buffer[size++] = vbat_mv;
    // distance
    uint16_t dist2 = distance;
    m_lora_app_data.buffer[size++] = dist2 >> 8;
    m_lora_app_data.buffer[size++] = dist2;

    m_lora_app_data.buffer[size++] = msgcount >> 8;
    m_lora_app_data.buffer[size++] = msgcount;
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
    msgcount++;
    digitalWrite(LED_GREEN, LOW);
  }
  xSemaphoreTake(taskEvent, 10);
}
