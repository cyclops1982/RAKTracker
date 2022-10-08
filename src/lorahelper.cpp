#include "lorahelper.h"
#include "ledhelper.h"
#include "main.h"

// uint8_t nodeDeviceEUI[8] = {0xAC, 0x1F, 0x09, 0xFF, 0xFE, 0x06, 0xBE, 0x44};
uint8_t nodeDeviceEUI[8] = {0xAC, 0x1F, 0x09, 0xFF, 0xFE, 0x08, 0xDD, 0xB1};
uint8_t nodeAppEUI[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t nodeAppKey[16] = {0x66, 0x7b, 0x90, 0x71, 0xa1, 0x72, 0x18, 0xd4, 0xcd, 0xb2, 0x13, 0x04, 0x3f, 0xb2, 0x6b, 0x7c};

lmh_callback_t lora_callbacks = {BatteryHelper::GetLoRaWanBattVal,
                                 BoardGetUniqueId,
                                 BoardGetRandomSeed,
                                 LoraHelper::lorawan_rx_handler,
                                 LoraHelper::lorawan_has_joined_handler,
                                 LoraHelper::lorawan_confirm_class_handler,
                                 LoraHelper::lorawan_join_failed_handler,
                                 LoraHelper::lorawan_unconf_finished,
                                 LoraHelper::lorawan_conf_finished};

void LoraHelper::lorawan_has_joined_handler(void)
{
    SERIAL_LOG("OTAA Mode, Network Joined!");
    lmh_error_status ret = lmh_class_request(LORAWAN_CLASS);
    if (ret == LMH_SUCCESS)
    {
        SERIAL_LOG("Class request status: %d\n", ret);
    }
}

void LoraHelper::lorawan_unconf_finished(void)
{
    SERIAL_LOG("TX finished");
}

void LoraHelper::lorawan_conf_finished(bool result)
{
    SERIAL_LOG("Confirmed TX %s\n", result ? "success" : "failed");
}

void LoraHelper::lorawan_join_failed_handler(void)
{
    SERIAL_LOG("OTAA join failed!");
    SERIAL_LOG("Check your EUI's and Keys's!");
    SERIAL_LOG("Check if a Gateway is in range!");
    LedHelper::BlinkHalt();
}

void LoraHelper::lorawan_rx_handler(lmh_app_data_t *app_data)
{
    SERIAL_LOG("LoRa Packet received on port %d, size:%d, rssi:%d, snr:%d, data:%s\n",
               app_data->port,
               app_data->buffsize,
               app_data->rssi,
               app_data->snr,
               app_data->buffer);

    switch (app_data->port)
    {
    case 3:
        // Port 3 switches the class
        if (app_data->buffsize == 1)
        {
            switch (app_data->buffer[0])
            {
            case 0:
                SERIAL_LOG("Request to switch to class A");
                lmh_class_request(CLASS_A);
                break;

            case 1:
                SERIAL_LOG("Request to switch to class B");
                lmh_class_request(CLASS_B);
                break;

            case 2:
                SERIAL_LOG("Request to switch to class C");
                lmh_class_request(CLASS_C);
                break;

            default:
                SERIAL_LOG("Failed to intepret appdata port 3 data.");
                break;
            }
        }
        break;
    case LORAWAN_APP_PORT:
        SERIAL_LOG("RECEIVED LORA DATA");
        // Copy the data into loop data buffer
        memcpy(g_rcvdLoRaData, app_data->buffer, app_data->buffsize);
        g_rcvdDataLen = app_data->buffsize;

        g_EventType = EventType::LoraDataReceived;

        if (g_taskEvent != NULL)
        {
            SERIAL_LOG("Waking up loop task: %d", g_EventType);
            xSemaphoreGive(g_taskEvent);
        }
        break;
    default:
        SERIAL_LOG("Received lora data on unsupported PORT");
        break;
    }
}

void LoraHelper::lorawan_confirm_class_handler(DeviceClass_t Class)
{
    SERIAL_LOG("switch to class %c done\n", "ABC"[Class]);
    // Informs the server that switch has occurred ASAP
    g_SendLoraData.buffsize = 0;
    g_SendLoraData.port = LORAWAN_APP_PORT;
    lmh_send(&g_SendLoraData, LMH_CONFIRMED_MSG);
}

void LoraHelper::SetDataRate(uint8_t datarate, bool adr)
{
    lmh_datarate_set(datarate, adr);
}

void LoraHelper::SetTXPower(uint8_t TXPower)
{
    lmh_tx_power_set(TXPower);
}

void LoraHelper::InitAndJoin()
{
    SERIAL_LOG("Init and Join LoraWAN");
#ifdef RAK4630
    lora_rak4630_init();
#elif RAK11310
    lora_rak11300_init();
#endif
    lmh_setDevEui(nodeDeviceEUI);
    lmh_setAppEui(nodeAppEUI);
    lmh_setAppKey(nodeAppKey);

    lmh_param_t lora_param_init = {LORAWAN_ADR_ON, LORAWAN_DATERATE, LORAWAN_PUBLIC_NETWORK, JOINREQ_NBTRIALS, LORAWAN_TX_POWER, LORAWAN_DUTYCYCLE_OFF};

    uint32_t err_code = lmh_init(&lora_callbacks, lora_param_init, true, CLASS_A, LORAMAC_REGION_EU868);
    if (err_code != 0)
    {
        SERIAL_LOG("lmh_init failed - %d\n", err_code);
        return;
    }

    // Start Join procedure
    lmh_join();

    while (lmh_join_status_get() != LMH_SET)
    {
        LedHelper::BlinkDelay(LED_BLUE, 100);
    }
}