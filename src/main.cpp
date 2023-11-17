#include <stdio.h>
#include <Wire.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
#include "batteryhelper.h"
#include "lorahelper.h"
#include "ledhelper.h"
#include "serialhelper.h"
#include "main.h"
#include "config.h"
#include "motion.h"

SoftwareTimer g_taskWakeupTimer;
SFE_UBLOX_GNSS g_GNSS;
uint16_t g_msgcount = 0;

SemaphoreHandle_t g_taskEvent = NULL;
EventType g_EventType = EventType::None;
uint8_t g_rcvdLoRaData[LORAWAN_BUFFER_SIZE];
uint8_t g_rcvdDataLen = 0;
bool g_lorawan_joined = false;

void periodicWakeup(TimerHandle_t unused)
{
  // Give the semaphore, so the loop task will wake up
  g_EventType = EventType::Timer;
  xSemaphoreGiveFromISR(g_taskEvent, pdFALSE);
}

void setup()
{
  delay(1000); // For whatever reason, some pins/things are not available at startup right away. So we wait for a bit. This also helps when we want to connect to console.
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
#endif
  SERIAL_LOG("Setup start.");

  if (!g_configParams.InitConfig())
  {
    LedHelper::BlinkHalt();
  }
  delay(1000);
  // Create semaphore for task handling.
  g_taskEvent = xSemaphoreCreateBinary();

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

  while (g_GNSS.begin() == false) // Attempt to re-connect
  {
    delay(500);
    SERIAL_LOG("Attempting to re-connect to u-blox GNSS...");
  }
  g_GNSS.setDynamicModel((dynModel)g_configParams.GetGNSSDynamicModel()); // turns out a Bike is like a sheep.
  g_GNSS.saveConfigSelective(VAL_CFG_SUBSEC_NAVCONF);
  g_GNSS.setI2COutput(COM_TYPE_UBX);
  g_GNSS.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT); // Save (only) the communications port settings to flash and BBR

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

  MotionHelper::InitMotionSensor(
      g_configParams.GetMotion1stThreshold(),
      g_configParams.GetMotion2ndThreshold(),
      g_configParams.GetMotion1stDuration(),
      g_configParams.GetMotion2ndDuration());

#ifndef LORAWAN_FAKE
  // Lora stuff
  LoraHelper::InitAndJoin(g_configParams.GetLoraDataRate(), g_configParams.GetLoraTXPower(), g_configParams.GetLoraADREnabled(),
                          g_configParams.GetLoraDevEUI(), g_configParams.GetLoraNodeAppEUI(), g_configParams.GetLoraAppKey());
#endif

  // Go into sleep mode
  xSemaphoreGive(g_taskEvent);
  g_EventType = EventType::Timer;
  g_taskWakeupTimer.begin(g_configParams.GetSleepTime0InSeconds() * 1000, periodicWakeup);
  g_taskWakeupTimer.start();
}

bool SendData()
{
#ifndef LORAWAN_FAKE
  if (!g_lorawan_joined)
  {
    SERIAL_LOG("Lora not joined yet while trying to send. Joining now.");
    LoraHelper::InitAndJoin(g_configParams.GetLoraDataRate(), g_configParams.GetLoraTXPower(), g_configParams.GetLoraADREnabled(),
                            g_configParams.GetLoraDevEUI(), g_configParams.GetLoraNodeAppEUI(), g_configParams.GetLoraAppKey());
  }
  if (g_lorawan_joined)
  {
    lmh_error_status loraSendState = lmh_send(&g_SendLoraData, (lmh_confirm)g_configParams.GetLoraRequireConfirmation());

    if (loraSendState == LMH_SUCCESS)
    {
      SERIAL_LOG("LoRaSend ok");
      return true;
    }
    else
    {
      SERIAL_LOG("LorRaSend failed: %d\n", loraSendState);
      return false;
    }
  }
  else
  {
    SERIAL_LOG("SKIPPING SEND - We are not joined to a network");
  }
  return false;
#else
  SERIAL_LOG("NOT SENDING lorawan packages as we have LORAWAN_FAKE set. Data that would be send:");
  for (uint8_t x = 0; x < g_SendLoraData.buffsize; x++)
  {
    SERIAL_LOG("%d: 0x%02X", x, g_SendLoraData.buffer[x]);
  }
  return true;
#endif
}

void handleReceivedMessage()
{
  /*  for (uint8_t i = 0; i < g_rcvdDataLen; i++)
  {
    char hexstr[5];
    sprintf(hexstr, "0x%02X", g_rcvdLoRaData[i]);
    SERIAL_LOG("DATA %d: %s", i, hexstr)
  }
  */
  g_configParams.SetConfig(g_rcvdLoRaData, g_rcvdDataLen);

  // Some parameters require some re-initialization, which is what we do here for those cases.
  size_t arraySize = 0;
  ConfigOption *configs = g_configParams.GetConfigs(&arraySize);
  for (uint8_t i = 0; i < g_rcvdDataLen; i++)
  {
    for (size_t x = 0; x < arraySize; x++)
    {
      ConfigOption conf = configs[x];
      if (conf.configType == g_rcvdLoRaData[i])
      {
        SERIAL_LOG("Updating setting %s", conf.name);
        switch (g_rcvdLoRaData[i])
        {
        case ConfigType::SleepTime0:
        case ConfigType::SleepTime1:
        case ConfigType::SleepTime2:
          SERIAL_LOG("Resetting sleeptimer to %u", g_configParams.GetSleepTime0InSeconds());
          SERIAL_LOG("Timers are now %u / %u / %u", g_configParams.GetSleepTime0InSeconds(), g_configParams.GetSleepTime1InSeconds(), g_configParams.GetSleepTime2InSeconds());
          // g_taskWakeupTimer.stop();
          g_taskWakeupTimer.setPeriod(g_configParams.GetSleepTime0InSeconds() * 1000);
          g_taskWakeupTimer.start();
          break;
        case ConfigType::GPSDynamicModel:
          SERIAL_LOG("Setting GNSS Dynamic Model to %u", g_configParams.GetGNSSDynamicModel());
          g_GNSS.setDynamicModel((dynModel)g_configParams.GetGNSSDynamicModel()); // turns out a Bike is like a sheep.
          g_GNSS.saveConfigSelective(VAL_CFG_SUBSEC_NAVCONF);
          break;
        case ConfigType::LORA_ADREnabled:
        case ConfigType::LORA_DataRate:
          SERIAL_LOG("Setting Lora DataRate to %u and ARD to %d", g_configParams.GetLoraDataRate(), g_configParams.GetLoraADREnabled());
          LoraHelper::SetDataRate(g_configParams.GetLoraDataRate(), g_configParams.GetLoraADREnabled());
          break;
        case ConfigType::LORA_TXPower:
          SERIAL_LOG("Setting Lora TX Power to %d", g_configParams.GetLoraTXPower());
          LoraHelper::SetTXPower(g_configParams.GetLoraTXPower());
          break;
        case ConfigType::MOTION_1stDuration:
        case ConfigType::MOTION_2ndDuration:
        case ConfigType::MOTION_1stThreshold:
        case ConfigType::MOTION_2ndThreshold:
          SERIAL_LOG("Setting motion sensor to 1/2nd threshold & 1/2nd duration: 0x%02X/0x%02X & 0x%02X/0x%02X",
                     g_configParams.GetMotion1stThreshold(), g_configParams.GetMotion2ndThreshold(), g_configParams.GetMotion1stDuration(), g_configParams.GetMotion2ndDuration());
          MotionHelper::InitMotionSensor(
              g_configParams.GetMotion1stThreshold(),
              g_configParams.GetMotion2ndThreshold(),
              g_configParams.GetMotion1stDuration(),
              g_configParams.GetMotion2ndDuration());
          break;
        }
        i += conf.sizeOfOption; // jump to the next one
        break;
      }
    }
  }
}

void doPeriodicUpdate()
{
  SERIAL_LOG("Doing GPSFix");
  digitalWrite(WB_IO2, HIGH);
  delay(500);

  uint32_t gpsStart = millis();
  bool gpsPVTStatus = g_GNSS.getPVT();
  while (!gpsPVTStatus)
  {
    gpsPVTStatus = g_GNSS.getPVT();
    SERIAL_LOG("PVT result: %d", gpsPVTStatus);
    delay(100);
  }
  byte gpsFixType = 0;

  uint32_t gnssTimeout = (uint32_t)g_configParams.GetGNSSFixTimeoutInSeconds() * 1000;
  while (1)
  {
    gpsFixType = g_GNSS.getFixType(); // Get the fix type

    if (gpsFixType == 3 && g_GNSS.getGnssFixOk())
    {
      SERIAL_LOG("FixType 3 and GnnsFixOK");
      break;
    }
    else
    {
      delay(500);
    }

    if ((millis() - gpsStart) > gnssTimeout)
    {
      SERIAL_LOG("GNSS fix timeout:  %u > %u", (millis() - gpsStart), gnssTimeout);
      break;
    }

    SERIAL_LOG("FixType: %d", gpsFixType);
  }
  uint16_t gpsTimeInSeconds = (uint16_t)((millis() - gpsStart) / 1000);
  int32_t gpsLat = g_GNSS.getLatitude();
  int32_t gpsLong = g_GNSS.getLongitude();
  uint8_t gpsSats = g_GNSS.getSIV();
  int16_t gpsAltitudeMSL = (g_GNSS.getAltitudeMSL() / 1000);

  SERIAL_LOG("GPS details: GPStime: %us; SATS: %d; FIXTYPE: %d; LAT: %d; LONG: %d; Alt: %d\r\n", gpsTimeInSeconds, gpsSats, gpsFixType, gpsLat, gpsLong, gpsAltitudeMSL);
  uint16_t vbat_mv = BatteryHelper::readVBAT();

  // We are done with the sensors, so we can turn them off
  digitalWrite(WB_IO2, LOW);

  // Create the lora message
  memset(g_SendLoraData.buffer, 0, LORAWAN_BUFFER_SIZE);
  int size = 0;
  g_SendLoraData.port = 2;
  g_SendLoraData.buffer[size++] = 0x03;
  if (MotionHelper::IsMotionEnabled())
  {
    g_SendLoraData.buffer[size++] = 0x06;
  }
  else
  {
    g_SendLoraData.buffer[size++] = 0x04;
  }

  g_SendLoraData.buffer[size++] = vbat_mv >> 8;
  g_SendLoraData.buffer[size++] = vbat_mv;

  g_SendLoraData.buffer[size++] = g_msgcount >> 8;
  g_SendLoraData.buffer[size++] = g_msgcount;

  g_SendLoraData.buffer[size++] = gpsTimeInSeconds >> 8;
  g_SendLoraData.buffer[size++] = gpsTimeInSeconds;

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

  // Add motionresult
  if (MotionHelper::IsMotionEnabled())
  {
    uint8_t motionresult = MotionHelper::GetMotionInterupts();
    g_SendLoraData.buffer[size++] = motionresult;

    SERIAL_LOG("Motion details: 0x%02X", motionresult)
    if ((motionresult & 0x10) == 0x10)
    {
      g_taskWakeupTimer.setPeriod(g_configParams.GetSleepTime2InSeconds() * 1000);
    }
    else if ((motionresult & 0x01) == 0x01)
    {
      g_taskWakeupTimer.setPeriod(g_configParams.GetSleepTime1InSeconds() * 1000);
    }
    else
    {
      g_taskWakeupTimer.setPeriod(g_configParams.GetSleepTime0InSeconds() * 1000);
    }
    g_taskWakeupTimer.start();
  }

  g_SendLoraData.buffsize = size;
  SendData();

  g_msgcount++;
};

void loop()
{
  if (xSemaphoreTake(g_taskEvent, portMAX_DELAY) == pdTRUE)
  {
    SERIAL_LOG("Running loop for EventType: %d", g_EventType);

#ifndef MAX_SAVE
    digitalWrite(LED_GREEN, HIGH); // indicate we're doing stuff
#endif
    switch (g_EventType)
    {
    case EventType::LoraDataReceived:
      handleReceivedMessage();
      break;
    case EventType::Timer:
      doPeriodicUpdate();
      break;
    case EventType::None:
    default:
      SERIAL_LOG("In loop, but without correct g_EventType")
      break;
    };

#ifndef MAX_SAVE
    digitalWrite(LED_GREEN, LOW);
#endif
  }
  xSemaphoreTake(g_taskEvent, 10);
}
