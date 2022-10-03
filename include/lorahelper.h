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
#define LORAWAN_BUFFER_SIZE 64                                         /**< buffer size of the data to be transmitted. */
#define LORAWAN_CLASS CLASS_C

static uint8_t g_sendLoraDataBuffer[LORAWAN_BUFFER_SIZE];            //< Lora user application data buffer.
static lmh_app_data_t g_SendLoraData = {g_sendLoraDataBuffer, 0, 0, 0, 0}; //< Lora user application data structure.


    

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
    LoraHelper() {};
};

#endif