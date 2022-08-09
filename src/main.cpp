#include <Arduino.h>
#include <SPI.h>
#include <stdio.h>
#include <Wire.h>
#include <vl53l0x_class.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
#include "batteryhelper.h"
#include "lorahelper.h"
#include "ledhelper.h"

#define SLEEPTIME (1000 * 300)

#define MAX_SAVE true
#define SERIAL_LOG(fmt, args...)  \
  {                               \
    if (MAX_SAVE == false)        \
    {                             \
      Serial.printf(fmt, ##args); \
      Serial.println();           \
    }                             \
  }

SemaphoreHandle_t g_taskEvent = NULL;
SoftwareTimer g_taskWakeupTimer;
VL53L0X g_vl53l0x(&Wire, WB_IO2);
SFE_UBLOX_GNSS g_GNSS;
uint16_t g_msgcount = 0;

void periodicWakeup(TimerHandle_t unused)
{
  // Give the semaphore, so the loop task will wake up
  xSemaphoreGiveFromISR(g_taskEvent, pdFALSE);
}

void wakeUpGNSS()
{
  digitalWrite(VAL_RXM_PMREQ_WAKEUPSOURCE_UARTRX, LOW);
  digitalWrite(VAL_RXM_PMREQ_WAKEUPSOURCE_UARTRX, HIGH);
  delay(50);
  digitalWrite(VAL_RXM_PMREQ_WAKEUPSOURCE_UARTRX, LOW);
}

uint16_t getDistance(void)
{
  g_vl53l0x.begin();
  if (g_vl53l0x.InitSensor(0x52) != VL53L0X_ERROR_NONE)
  {
    SERIAL_LOG("Init g_vl53l0x failed...");
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
  delay(3000);

  // Initialize serial for output.
#if !MAX_SAVE
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
#endif

  // Setup/start the wire that we use for the Sensor.
  Wire.begin();

  // Start up the GPS
  pinMode(WB_IO2, OUTPUT);
  digitalWrite(WB_IO2, LOW);
  delay(100);
  digitalWrite(WB_IO2, HIGH);
  delay(100);
  pinMode(VAL_RXM_PMREQ_WAKEUPSOURCE_UARTRX, OUTPUT);
  digitalWrite(VAL_RXM_PMREQ_WAKEUPSOURCE_UARTRX, LOW);

  if (g_GNSS.begin() == false)
  {
    SERIAL_LOG("Ublox GPS not detected at default I2C address. Please check wiring. Halting.");
    LedHelper::BlinkHalt();
  }

  // Setup the GPS.
  g_GNSS.setDynamicModel(DYN_MODEL_WRIST);
  g_GNSS.setI2COutput(COM_TYPE_UBX);
  g_GNSS.saveConfiguration();

  LoraHelper::InitAndJoin();

  // Go into sleep mode
  g_GNSS.powerOffWithInterrupt((SLEEPTIME * 2), VAL_RXM_PMREQ_WAKEUPSOURCE_UARTRX);
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

    // Wake up the GNSS module and then do some other stuff (while that gets a fix)
    wakeUpGNSS();
    uint32_t gpsStart = millis();
    uint16_t distance = getDistance();
    uint16_t vbat_mv = BatteryHelper::readVBAT();

    byte gpsFixType = g_GNSS.getFixType();
    while (gpsFixType != 3 && ((millis() - gpsStart) < (1000 * 300)))
    {
#if !MAX_SAVE
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
      LedHelper::BlinkDelay(LED_BLUE, 250);
      gpsFixType = g_GNSS.getFixType(); // Get the fix type
    }
    uint16_t gpsTime = millis() - gpsStart;
    uint32_t gpsLat = g_GNSS.getLatitude();
    uint32_t gpsLong = g_GNSS.getLongitude();
    uint8_t gpsSats = g_GNSS.getSIV();
    uint32_t gpsAltitudeMSL = g_GNSS.getAltitudeMSL();

    g_GNSS.powerOffWithInterrupt((SLEEPTIME * 2), VAL_RXM_PMREQ_WAKEUPSOURCE_UARTRX);
    SERIAL_LOG("GPS details: GPStime: %dms; SATS: %d; FIXTYPE: %d; LAT: %d; LONG: %d;\r\n", gpsTime, gpsSats, gpsFixType, gpsLat, gpsLong);

    // Create the lora message
    memset(m_lora_app_data.buffer, 0, LORAWAN_APP_DATA_BUFF_SIZE);
    int size = 0;
    m_lora_app_data.port = 2;
    m_lora_app_data.buffer[size++] = 0x02; // device
    m_lora_app_data.buffer[size++] = 0x03; // msg version

    m_lora_app_data.buffer[size++] = vbat_mv >> 8;
    m_lora_app_data.buffer[size++] = vbat_mv;

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

    m_lora_app_data.buffer[size++] = gpsAltitudeMSL >> 24;
    m_lora_app_data.buffer[size++] = gpsAltitudeMSL >> 16;
    m_lora_app_data.buffer[size++] = gpsAltitudeMSL >> 8;
    m_lora_app_data.buffer[size++] = gpsAltitudeMSL;

    m_lora_app_data.buffsize = size;

    lmh_error_status error = lmh_send_blocking(&m_lora_app_data, LMH_CONFIRMED_MSG, 15000);
#if !MAX_SAVE
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
