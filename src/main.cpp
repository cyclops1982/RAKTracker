#include <Arduino.h>
#include <SPI.h>
#include <stdio.h>
#include <Wire.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
#include "batteryhelper.h"
#include "lorahelper.h"
#include "ledhelper.h"
#include "serialhelper.h"

#define SLEEPTIME (1000 * 60 * 4)

SemaphoreHandle_t g_taskEvent = NULL;
SoftwareTimer g_taskWakeupTimer;
SFE_UBLOX_GNSS g_GNSS;
uint16_t g_msgcount = 0;

void periodicWakeup(TimerHandle_t unused)
{
  // Give the semaphore, so the loop task will wake up
  xSemaphoreGiveFromISR(g_taskEvent, pdFALSE);
}
void setup()
{
  delay(1000); // For whatever reason, some pins/things are not available at startup right away. So we wait 3 seconds for stuff to warm up or something
  LedHelper::init();
  SERIAL_LOG("Setup start.");
  delay(1000);

  // Turn on power to sensors
  pinMode(WB_IO2, OUTPUT);
  digitalWrite(WB_IO2, HIGH);
  // interrupt pin for GNSS module
  // pinMode(PIN_SERIAL2_RX, OUTPUT);
  // digitalWrite(PIN_SERIAL2_RX, LOW);

  delay(1000);

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
  g_GNSS.setDynamicModel(DYN_MODEL_WRIST);
  g_GNSS.setI2COutput(COM_TYPE_UBX);
  g_GNSS.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT); // Save (only) the communications port settings to flash and BBR

  // We should also disable SBAS and IMES via UBX-CFG-GNSS
  // We should also turn of time-pulses via UBX-CFG-TP5
  // We should be doing Power Save Mode ON/OFF (PSMOO) Operation
  if (g_GNSS.powerSaveMode(true) == false)
  {
    SERIAL_LOG("POwerSave not supported, or couldn't be set.");
  }

  uint8_t powerSave = g_GNSS.getPowerSaveMode();
  if (powerSave == 255)
  {
    SERIAL_LOG("Failed to go into powersave mode.");
    LedHelper::BlinkHalt();
  }
  SERIAL_LOG("PowerSave on GNSS: %d", powerSave);
  g_GNSS.saveConfigSelective(VAL_CFG_SUBSEC_RXMCONF); // Store the fact that we want powersave mode

  // Lora stuff
  LoraHelper::InitAndJoin();

  // Go into sleep mode
  g_taskEvent = xSemaphoreCreateBinary();
  xSemaphoreGive(g_taskEvent);
  g_taskWakeupTimer.begin(SLEEPTIME, periodicWakeup);
  g_taskWakeupTimer.start();
}

void loop()
{
  if (xSemaphoreTake(g_taskEvent, portMAX_DELAY) == pdTRUE)
  {
    SERIAL_LOG("Within loop...");
    digitalWrite(LED_GREEN, HIGH); // indicate we're doing stuff
    // digitalWrite(PIN_SERIAL2_RX, HIGH);
    // delay(1000);
    // digitalWrite(PIN_SERIAL2_RX, LOW);

    uint32_t gpsStart = millis();
    bool gpsPVTStatus = g_GNSS.getPVT();
    while (!gpsPVTStatus)
    {
      gpsPVTStatus = g_GNSS.getPVT();
      SERIAL_LOG("PVT result: %d", gpsPVTStatus);
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

      if ((millis() - gpsStart) > (1000 * 120))
      {
        SERIAL_LOG("GNSS fix timeout");
        break;
      }

      SERIAL_LOG("FIxType: %d", gpsFixType);
    }
    uint16_t gpsTime = millis() - gpsStart;
    uint32_t gpsLat = g_GNSS.getLatitude();
    uint32_t gpsLong = g_GNSS.getLongitude();
    uint8_t gpsSats = g_GNSS.getSIV();
    int16_t gpsAltitudeMSL = (g_GNSS.getAltitudeMSL() / 1000);

    g_GNSS.powerOff(SLEEPTIME);
    //, VAL_RXM_PMREQ_WAKEUPSOURCE_UARTRX, true);
    SERIAL_LOG("GPS details: GPStime: %dms; SATS: %d; FIXTYPE: %d; LAT: %d; LONG: %d;\r\n", gpsTime, gpsSats, gpsFixType, gpsLat, gpsLong);
    uint16_t vbat_mv = BatteryHelper::readVBAT();

    // We are done with the sensors, so we can turn them off

    // Create the lora message
    memset(m_lora_app_data.buffer, 0, LORAWAN_APP_DATA_BUFF_SIZE);
    int size = 0;
    m_lora_app_data.port = 2;
    m_lora_app_data.buffer[size++] = 0x02; // device
    m_lora_app_data.buffer[size++] = 0x03; // msg version

    m_lora_app_data.buffer[size++] = vbat_mv >> 8;
    m_lora_app_data.buffer[size++] = vbat_mv;
    m_lora_app_data.buffer[size++] = 0;
    m_lora_app_data.buffer[size++] = 0;

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

    m_lora_app_data.buffer[size++] = gpsAltitudeMSL >> 8;
    m_lora_app_data.buffer[size++] = gpsAltitudeMSL;

    m_lora_app_data.buffsize = size;

    lmh_error_status loraSendState = LMH_ERROR;
    loraSendState = lmh_send_blocking(&m_lora_app_data, LMH_CONFIRMED_MSG, 15000);
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
    g_msgcount++;
    digitalWrite(LED_GREEN, LOW);
  }
  xSemaphoreTake(g_taskEvent, 10);
}
