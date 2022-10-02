#include <Arduino.h>
#include <SPI.h>
#include <stdio.h>
#include <Wire.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
#include "batteryhelper.h"
#include "lorahelper.h"
#include "ledhelper.h"
#include "serialhelper.h"

#define SLEEPTIME (1000 * 60 * 3)
SoftwareTimer g_taskWakeupTimer;
SFE_UBLOX_GNSS g_GNSS;
uint16_t g_msgcount = 0;

void periodicWakeup(TimerHandle_t unused)
{
  // Give the semaphore, so the loop task will wake up
  g_EventType = EventTypeEnum::Timer;
  xSemaphoreGiveFromISR(g_taskEvent, pdFALSE);
}
void setup()
{
  delay(1000); // For whatever reason, some pins/things are not available at startup right away. So we wait 3 seconds for stuff to warm up or something
  LedHelper::init();
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
  SERIAL_LOG("Setup start.");
  delay(1000);

  // Turn on power to sensors
  pinMode(WB_IO2, OUTPUT);
  digitalWrite(WB_IO2, HIGH);
  delay(100);

  // Setup/start the wire that we use for the Sensor.
  Wire.begin();

  // Get all GPS stuff fired up
  if (g_GNSS.begin() == false)
  {
    SERIAL_LOG("Ublox GPS not detected at default I2C address. Please check wiring. Halting.");
    LedHelper::BlinkHalt();
  }

  SERIAL_LOG("Found GNSS with Protocol version: %d.%d", g_GNSS.getProtocolVersionHigh(), g_GNSS.getProtocolVersionLow());
  g_GNSS.factoryReset();
  // g_GNSS.enableDebugging();

  while (g_GNSS.begin() == false) // Attempt to re-connect
  {
    delay(500);
    SERIAL_LOG("Attempting to re-connect to u-blox GNSS...");
  }
  g_GNSS.setDynamicModel(DYN_MODEL_BIKE); // turns out a Bike is like a sheep.
  g_GNSS.setI2COutput(COM_TYPE_UBX);
  g_GNSS.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT); // Save (only) the communications port settings to flash and BBR
  g_GNSS.saveConfigSelective(VAL_CFG_SUBSEC_NAVCONF);

  // We should also disable SBAS and IMES via UBX-CFG-GNSS
  // We should also turn of time-pulses via UBX-CFG-TP5
  // We should be doing Power Save Mode ON/OFF (PSMOO) Operation
  if (g_GNSS.powerSaveMode(false) == false)
  {
    SERIAL_LOG("PowerSave not supported, or couldn't be set.");
  }

  uint8_t powerSave = g_GNSS.getPowerSaveMode();
  if (powerSave == 255)
  {
    SERIAL_LOG("Failed to retrieve powersave mode.");
    LedHelper::BlinkHalt();
  }
  SERIAL_LOG("PowerSave on GNSS: %d", powerSave);
  g_GNSS.saveConfigSelective(VAL_CFG_SUBSEC_RXMCONF); // Store the fact that we want powersave mode

  // Lora stuff
  LoraHelper::InitAndJoin();

  // Go into sleep mode

  xSemaphoreGive(g_taskEvent);
  g_taskWakeupTimer.begin(SLEEPTIME, periodicWakeup);
  g_taskWakeupTimer.start();
}

void handleReceivedMessage()
{
}

void doGPSFix()
{
  digitalWrite(WB_IO2, HIGH);
  delay(1000);

  uint32_t gpsStart = millis();
  bool gpsPVTStatus = g_GNSS.getPVT();
  while (!gpsPVTStatus)
  {
    gpsPVTStatus = g_GNSS.getPVT();
    SERIAL_LOG("PVT result: %d", gpsPVTStatus);
    delay(100);
  }
  byte gpsFixType = 0;
  while (1)
  {
    // LedHelper::BlinkDelay(LED_BLUE, 250);
    gpsFixType = g_GNSS.getFixType(); // Get the fix type

    if (gpsFixType == 3 && g_GNSS.getGnssFixOk())
    {
      SERIAL_LOG("FixType 3 and GnnsFixOK");
      break;
    }
    else
    {
      delay(100);
    }

    if ((millis() - gpsStart) > (1000 * 120))
    {
      SERIAL_LOG("GNSS fix timeout");
      break;
    }

    SERIAL_LOG("FIxType: %d", gpsFixType);
  }
  uint32_t gpsTime = millis() - gpsStart;
  uint32_t gpsLat = g_GNSS.getLatitude();
  uint32_t gpsLong = g_GNSS.getLongitude();
  uint8_t gpsSats = g_GNSS.getSIV();
  int16_t gpsAltitudeMSL = (g_GNSS.getAltitudeMSL() / 1000);

  SERIAL_LOG("GPS details: GPStime: %ums; SATS: %d; FIXTYPE: %d; LAT: %u; LONG: %u; Alt: %d\r\n", gpsTime, gpsSats, gpsFixType, gpsLat, gpsLong, gpsAltitudeMSL);
  uint16_t vbat_mv = BatteryHelper::readVBAT();

  // Create the lora message
  memset(g_SendLoraData.buffer, 0, LORAWAN_APP_DATA_BUFF_SIZE);
  int size = 0;
  g_SendLoraData.port = 2;
  g_SendLoraData.buffer[size++] = 0x02; // device
  g_SendLoraData.buffer[size++] = 0x03; // msg version

  g_SendLoraData.buffer[size++] = vbat_mv >> 8;
  g_SendLoraData.buffer[size++] = vbat_mv;
  g_SendLoraData.buffer[size++] = 0;
  g_SendLoraData.buffer[size++] = 0;

  g_SendLoraData.buffer[size++] = g_msgcount >> 8;
  g_SendLoraData.buffer[size++] = g_msgcount;
  uint16_t gpsTimeThing = gpsTime;
  SERIAL_LOG("GPS Time in packet: %d", gpsTimeThing);
  g_SendLoraData.buffer[size++] = gpsTimeThing >> 8;
  g_SendLoraData.buffer[size++] = gpsTimeThing;

  g_SendLoraData.buffer[size++] = gpsSats;
  g_SendLoraData.buffer[size++] = gpsFixType;

  g_SendLoraData.buffer[size++] = gpsLat >> 24;
  g_SendLoraData.buffer[size++] = gpsLat >> 16;
  g_SendLoraData.buffer[size++] = gpsLat >> 8;
  g_SendLoraData.buffer[size++] = gpsLat;

  g_SendLoraData.buffer[size++] = gpsLong >> 24;
  g_SendLoraData.buffer[size++] = gpsLong >> 16;
  g_SendLoraData.buffer[size++] = gpsLong >> 8;
  g_SendLoraData.buffer[size++] = gpsLong;

  g_SendLoraData.buffer[size++] = gpsAltitudeMSL >> 8;
  g_SendLoraData.buffer[size++] = gpsAltitudeMSL;

  g_SendLoraData.buffsize = size;

  lmh_error_status loraSendState = LMH_ERROR;
  loraSendState = lmh_send_blocking(&g_SendLoraData, LMH_CONFIRMED_MSG, 15000);
#if !MAX_SAVE
  if (loraSendState == LMH_SUCCESS)
  {
    Serial.println("lmh_send ok");
  }
  else
  {
    Serial.printf("lmh_send failed: %d\n", loraSendState);
  }
#endif
  // We are done with the sensors, so we can turn them off
  digitalWrite(WB_IO2, LOW);

  g_msgcount++;
};

void loop()
{
  if (xSemaphoreTake(g_taskEvent, portMAX_DELAY) == pdTRUE)
  {
#if MAX_SAVE == false
    digitalWrite(LED_GREEN, HIGH); // indicate we're doing stuff
#endif
    switch (g_EventType)
    {
    case EventTypeEnum::LoraDataReceived:
      handleReceivedMessage();
      break;

    case EventTypeEnum::Timer:
    case EventTypeEnum::None:
    default:
      doGPSFix();
      break;
    };

#if MAX_SAVE == false
    digitalWrite(LED_GREEN, LOW);
#endif
  }
  xSemaphoreTake(g_taskEvent, 10);
}
