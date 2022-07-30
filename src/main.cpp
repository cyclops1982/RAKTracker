/**
   @file RAK12014_Distance_Detection.ino
   @author rakwireless.com
   @brief Distance detection by laser
   @version 0.1
   @date 2021-8-28
   @copyright Copyright (c) 2020
**/

#include <Arduino.h>
//#include "LoRaWan-Arduino.h" //http://librarymanager/All#SX126x
//#include <SPI.h>
//#include <stdio.h>
//#include "mbed.h"
//#include "rtos.h"

#include <Wire.h>
#include <vl53l0x_class.h>

VL53L0X sensor_vl53l0x(&Wire, WB_IO2);

// uint8_t nodeDeviceEUI[8] = {0xAC, 0x1F, 0x09, 0xFF, 0xFE, 0x06, 0xBE, 0x44};
// uint8_t nodeAppEUI[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
// uint8_t nodeAppKey[16] = {0x66, 0x7b, 0x90, 0x71, 0xa1, 0x72, 0x18, 0xd4, 0xcd, 0xb2, 0x13, 0x04, 0x3f, 0xb2, 0x6b, 0x7c};

// static void lorawan_has_joined_handler(void);
// static void lorawan_join_failed_handler(void);
// static void lorawan_rx_handler(lmh_app_data_t *app_data);
// static void lorawan_confirm_class_handler(DeviceClass_t Class);
// void lorawan_unconf_finished(void);
// void lorawan_conf_finished(bool result);

// //#define SCHED_MAX_EVENT_DATA_SIZE APP_TIMER_SCHED_EVENT_DATA_SIZE /**< Maximum size of scheduler events. */
// //#define SCHED_QUEUE_SIZE 60                     /**< Maximum number of events in the scheduler queue. */
// #define LORAWAN_DATERATE DR_0       /*LoRaMac datarates definition, from DR_0 to DR_5*/
// #define LORAWAN_TX_POWER TX_POWER_5 /*LoRaMac tx power definition, from TX_POWER_0 to TX_POWER_15*/
// #define JOINREQ_NBTRIALS 3
// #define LORAWAN_APP_PORT 2
// #define LORAWAN_APP_DATA_BUFF_SIZE 64                                         /**< buffer size of the data to be transmitted. */
// static uint8_t m_lora_app_data_buffer[LORAWAN_APP_DATA_BUFF_SIZE];            //< Lora user application data buffer.
// static lmh_app_data_t m_lora_app_data = {m_lora_app_data_buffer, 0, 0, 0, 0}; //< Lora user application data structure.

// static lmh_callback_t lora_callbacks = {BoardGetBatteryLevel, BoardGetUniqueId, BoardGetRandomSeed,
//                                         lorawan_rx_handler, lorawan_has_joined_handler, lorawan_confirm_class_handler,
//                                         lorawan_join_failed_handler, lorawan_unconf_finished, lorawan_conf_finished};

// static lmh_param_t lora_param_init = {LORAWAN_ADR_ON, LORAWAN_DATERATE, LORAWAN_PUBLIC_NETWORK, JOINREQ_NBTRIALS, LORAWAN_TX_POWER, LORAWAN_DUTYCYCLE_OFF};

#define PIN_VBAT WB_A0

uint32_t vbat_pin = PIN_VBAT;

#define VBAT_MV_PER_LSB (0.806F)   // 3.0V ADC range and 12 - bit ADC resolution = 3300mV / 4096
#define VBAT_DIVIDER (0.6F)        // 1.5M + 1M voltage divider on VBAT = (1.5M / (1M + 1.5M))
#define VBAT_DIVIDER_COMP (1.846F) //  // Compensation factor for the VBAT divider

#define REAL_VBAT_MV_PER_LSB (VBAT_DIVIDER_COMP * VBAT_MV_PER_LSB)

void setup()
{
  pinMode(WB_IO2, OUTPUT);
  digitalWrite(WB_IO2, HIGH);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  int status;

  // Initialize serial for output.
  Serial.begin(115200);

  // Initialize I2C bus.
  Wire.begin();

  // Configure VL53L0X component.
  sensor_vl53l0x.begin();

  // // Switch off VL53L0X component.
  sensor_vl53l0x.VL53L0X_Off();

  // // Initialize VL53L0X component.
  status = sensor_vl53l0x.InitSensor(0x52);
  if (status)
  {
    Serial.println("Init sensor_vl53l0x failed...");
  }
  // Something needed for the battery stuff
  analogReadResolution(12); // Can be 8, 10, 12 or 14

  Serial.println("Setup done");
  // Initialize LoRa chip.
  // lora_rak11300_init();
  // lmh_setDevEui(nodeDeviceEUI);
  // lmh_setAppEui(nodeAppEUI);
  // lmh_setAppKey(nodeAppKey);

  // uint32_t err_code = lmh_init(&lora_callbacks, lora_param_init, true, CLASS_A, LORAMAC_REGION_EU868);
  // if (err_code != 0)
  //{
  //    Serial.printf("lmh_init failed - %d\n", err_code);
  // return;
  //  }

  // Start Join procedure
  // lmh_join();
}

/**
 * @brief Get RAW Battery Voltage
 */
uint16_t readVBAT(void)
{
  unsigned int sum = 0, average_value = 0;
  unsigned int read_temp[10] = {0};
  unsigned char i = 0;
  unsigned int adc_max = 0;
  unsigned int adc_min = 4095;
  average_value = analogRead(vbat_pin);
  for (i = 0; i < 10; i++)
  {
    read_temp[i] = analogRead(vbat_pin);
    if (read_temp[i] < adc_min)
    {
      adc_min = read_temp[i];
    }
    if (read_temp[i] > adc_max)
    {
      adc_max = read_temp[i];
    }
    sum = sum + read_temp[i];
  }
  average_value = (sum - adc_max - adc_min) >> 3;

  // Convert the raw value to compensated mv, taking the resistor-
  // divider into account (providing the actual LIPO voltage)
  // ADC range is 0..3300mV and resolution is 12-bit (0..4095)
  uint16_t volt = average_value * REAL_VBAT_MV_PER_LSB;

  return volt;
}

/**
 * @brief Convert from raw mv to percentage
 * @param mvolts
 *    RAW Battery Voltage
 */
uint8_t mvToPercent(uint16_t mvolts)
{
  if (mvolts < 3300)
    return 0;

  if (mvolts < 3600)
  {
    mvolts -= 3300;
    return mvolts / 30;
  }

  mvolts -= 3600;
  return 10 + (mvolts * 0.15F); // thats mvolts /6.66666666
}

uint16_t msgcount = 0;
void loop()
{
  uint32_t distance;
  int status;

  status = sensor_vl53l0x.GetDistance(&distance);

  if (status == VL53L0X_ERROR_NONE)
  {
    // Output data.
    char report[64];
    snprintf(report, sizeof(report), "Distance: %ldmm", distance);
    Serial.println(report);

    if (distance < 50)
    {
      digitalWrite(LED_BUILTIN, HIGH);
    }
    else
    {
      digitalWrite(LED_BUILTIN, LOW);
    }

    // if (lmh_join_status_get() != LMH_SET)
    // {
    //   Serial.println("We have not joined lora yet...");
    //   return;
    // }

    // memset(m_lora_app_data.buffer, 0, LORAWAN_APP_DATA_BUFF_SIZE);
    // int size = 0;
    // m_lora_app_data.port = 2;
    // m_lora_app_data.buffer[size++] = 0x02; // device
    // m_lora_app_data.buffer[size++] = 0x02; // msg version

    // // bat voltage
    // m_lora_app_data.buffer[size++] = 0;
    // m_lora_app_data.buffer[size++] = 0;
    // // distance
    // uint16_t dist2 = distance;
    // m_lora_app_data.buffer[size++] = dist2 >> 8;
    // m_lora_app_data.buffer[size++] = dist2;

    // m_lora_app_data.buffer[size++] = msgcount >> 8;
    // m_lora_app_data.buffer[size++] = msgcount;
    // m_lora_app_data.buffsize = size;

    // lmh_error_status error = lmh_send(&m_lora_app_data, LMH_CONFIRMED_MSG);
    // if (error == LMH_SUCCESS)
    // {
    //   Serial.println("lmh_send ok");
    // }
    // else
    // {
    //   Serial.printf("lmh_send failed: %d\n", error);
    // }
    msgcount++;
  }
  uint16_t vbat_mv = readVBAT();

  // Convert from raw mv to percentage (based on LIPO chemistry)
  uint8_t vbat_per = mvToPercent(vbat_mv);
  Serial.printf("percentage: %d uint: %d\r\n", vbat_per, vbat_mv);

  delay(500);
}

// void lorawan_has_joined_handler(void)
// {

//   Serial.println("OTAA Mode, Network Joined!");
//   lmh_error_status ret = lmh_class_request(CLASS_A);
//   if (ret == LMH_SUCCESS)
//   {
//     Serial.printf("Class request status: %d\n", ret);
//   }
// }

// void lorawan_unconf_finished(void)
// {
//   Serial.println("TX finished");
// }

// void lorawan_conf_finished(bool result)
// {
//   Serial.printf("Confirmed TX %s\n", result ? "success" : "failed");
// }

// /**@brief LoRa function for handling OTAA join failed
//  */
// static void lorawan_join_failed_handler(void)
// {
//   Serial.println("OTAA join failed!");
//   Serial.println("Check your EUI's and Keys's!");
//   Serial.println("Check if a Gateway is in range!");
// }

// void lorawan_rx_handler(lmh_app_data_t *app_data)
// {
//   Serial.printf("LoRa Packet received on port %d, size:%d, rssi:%d, snr:%d, data:%s\n",
//                 app_data->port, app_data->buffsize, app_data->rssi, app_data->snr, app_data->buffer);
// }

// void lorawan_confirm_class_handler(DeviceClass_t Class)
// {
//   Serial.printf("switch to class %c done\n", "ABC"[Class]);
//   // Informs the server that switch has occurred ASAP
//   m_lora_app_data.buffsize = 0;
//   m_lora_app_data.port = LORAWAN_APP_PORT;
//   lmh_send(&m_lora_app_data, LMH_CONFIRMED_MSG);
// }
