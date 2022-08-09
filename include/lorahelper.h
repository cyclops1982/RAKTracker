#ifndef LORAHELPER_H
#define LORAHELPER_H
#include <Arduino.h>
#include <LoRaWan-Arduino.h>
#include "batteryhelper.h"
#include "serialhelper.h"

//#define SCHED_MAX_EVENT_DATA_SIZE APP_TIMER_SCHED_EVENT_DATA_SIZE /**< Maximum size of scheduler events. */
//#define SCHED_QUEUE_SIZE 60                     /**< Maximum number of events in the scheduler queue. */
#define LORAWAN_DATERATE DR_0       /*LoRaMac datarates definition, from DR_0 to DR_5*/
#define LORAWAN_TX_POWER TX_POWER_5 /*LoRaMac tx power definition, from TX_POWER_0 to TX_POWER_15*/
#define JOINREQ_NBTRIALS 3
#define LORAWAN_APP_PORT 2
#define LORAWAN_APP_DATA_BUFF_SIZE 64                                         /**< buffer size of the data to be transmitted. */
static uint8_t m_lora_app_data_buffer[LORAWAN_APP_DATA_BUFF_SIZE];            //< Lora user application data buffer.
static lmh_app_data_t m_lora_app_data = {m_lora_app_data_buffer, 0, 0, 0, 0}; //< Lora user application data structure.

class LoraHelper
{
public:
    static void lorawan_has_joined_handler(void);
    static void lorawan_join_failed_handler(void);
    static void lorawan_rx_handler(lmh_app_data_t *app_data);
    static void lorawan_confirm_class_handler(DeviceClass_t Class);
    static void lorawan_unconf_finished(void);
    static void lorawan_conf_finished(bool result);
    static void InitAndJoin();

private:
    LoraHelper(){};
};

static lmh_callback_t lora_callbacks = {BatteryHelper::GetLoRaWanBattVal,
                                        BoardGetUniqueId,
                                        BoardGetRandomSeed,
                                        LoraHelper::lorawan_rx_handler,
                                        LoraHelper::lorawan_has_joined_handler,
                                        LoraHelper::lorawan_confirm_class_handler,
                                        LoraHelper::lorawan_join_failed_handler,
                                        LoraHelper::lorawan_unconf_finished,
                                        LoraHelper::lorawan_conf_finished};

static lmh_param_t lora_param_init = {LORAWAN_ADR_ON, LORAWAN_DATERATE, LORAWAN_PUBLIC_NETWORK, JOINREQ_NBTRIALS, LORAWAN_TX_POWER, LORAWAN_DUTYCYCLE_OFF};

#endif