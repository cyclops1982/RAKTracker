#ifndef CONFIG_H
#define CONFIG_H
#include <Arduino.h>
#include <lorahelper.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>


// These config settings can be updated remotely.
enum ConfigType
{
    SleepTime = 0x01,
    GPSDynamicModel = 0x20,
    GPSFixTimeout = 0x21,
    LORA_TXPower = 0x40,
    LORA_DataRate = 0x41,
    LORA_ADREnabled = 0x42,
    LORA_RequireConfirmation = 0x43,
    MOTION_1stThreshold = 0x51,
    MOTION_2ndThreshold = 0x52,
    MOTION_1stDuration = 0x53,
    MOTION_2ndDuration = 0x54
};

struct ConfigOption
{
    const char *name;
    ConfigType configType;
    size_t sizeOfOption;
    void *value;
    void (*setfunc)(const ConfigOption *opt, uint8_t *arr);
};

struct ConfigurationParameters
{
    // We use these config parameters throughout our code. They are hardcoded here because
    // the rak thigns don't have permanent storage, and thus we need to code them into the software.
    // The below values are therefor also the default values.
    // Some of the settings can be updated remotely.  See ConfigType above and/or g_configs below for a list
    // of those.
    //Sheeptracker 1
    uint8_t _loraDevEUI[8] = {0xAC, 0x1F, 0x09, 0xFF, 0xFE, 0x08, 0xDD, 0xB1};
    uint8_t _loraNodeAppKey[16] = {0x66, 0x7b, 0x90, 0x71, 0xa1, 0x72, 0x18, 0xd4, 0xcd, 0xb2, 0x13, 0x04, 0x3f, 0xb2, 0x6b, 0x7c};

    // SheepTracker 2
    //uint8_t _loraDevEUI[8] = {0xAC, 0x1F, 0x09, 0xFF, 0xFE, 0x08, 0xF5, 0x2B};
    //uint8_t _loraNodeAppKey[16] = {0x4b, 0xbb, 0x43, 0xef, 0x4b, 0xc6, 0x46, 0x22, 0x1b, 0x0d, 0xcb, 0xe0, 0x44, 0x54, 0xb6, 0x1a};


    // Config settings
    uint16_t _sleeptime = 180;      // in seconds
    uint16_t _gnssFixTimeout = 90; // in seconds
    uint8_t _gnssDynamicModel = dynModel::DYN_MODEL_PEDESTRIAN;

    uint8_t _motion1stThreshold = 0x02;
    uint8_t _motion2ndThreshold = 0x00;
    uint8_t _motion1stDuration = 0x01;
    uint8_t _motion2ndDuration = 0x00;


    int8_t _loraDataRate = DR_2;
    int8_t _loraTXPower = TX_POWER_2;
    bool _loraADREnabled = false;
    bool _loraRequireConfirmation = false;
    uint8_t _loraNodeAppEUI[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    
    
    static void SetUint32(const ConfigOption *option, uint8_t *arr);
    static void SetUint16(const ConfigOption *option, uint8_t *arr);
    static void SetUint8(const ConfigOption *option, uint8_t *arr);
    static void SetInt8(const ConfigOption *option, uint8_t *arr);
    static void SetBool(const ConfigOption *option, uint8_t *arr);

public:
    uint32_t GetSleepTimeInSeconds() { return _sleeptime; }
    uint16_t GetGNSSFixTimeoutInSeconds() { return _gnssFixTimeout; }
    // TODO: make this return `dynModel`. Requires a new Setmethod
    uint8_t GetGNSSDynamicModel() { return _gnssDynamicModel; }

    uint8_t GetLoraTXPower() { return _loraTXPower; }
    uint8_t GetLoraDataRate() { return _loraDataRate; }
    bool GetLoraADREnabled() { return _loraADREnabled; }
    uint8_t *GetLoraDevEUI() { return _loraDevEUI; }
    uint8_t *GetLoraNodeAppEUI() { return _loraNodeAppEUI; }
    uint8_t *GetLoraAppKey() { return _loraNodeAppKey; }
    bool GetLoraRequireConfirmation() { return _loraRequireConfirmation; }

    uint8_t GetMotion1stDuration() { return _motion1stDuration; }
    uint8_t GetMotion2ndDuration() { return _motion2ndDuration; }
    
    uint8_t GetMotion1stThreshold() { return _motion1stThreshold; }
    uint8_t GetMotion2ndThreshold() { return _motion2ndThreshold; }
    void SetConfig(uint8_t *array, uint8_t length);

} g_configParams;

static const ConfigOption g_configs[] = {
    {"Sleep time between GPS fixes (in seconds)", ConfigType::SleepTime, sizeof(g_configParams._sleeptime), &g_configParams._sleeptime, ConfigurationParameters::SetUint16},
    {"GPS - Fix timeout (in seconds)", ConfigType::GPSFixTimeout, sizeof(g_configParams._gnssFixTimeout), &g_configParams._gnssFixTimeout, ConfigurationParameters::SetUint16},
    {"GPS - Dynamic Model", ConfigType::GPSDynamicModel, sizeof(g_configParams._gnssDynamicModel), &g_configParams._gnssDynamicModel, ConfigurationParameters::SetUint8},
    {"LoraWAN - TX Power", ConfigType::LORA_TXPower, sizeof(g_configParams._loraTXPower), &g_configParams._loraTXPower, ConfigurationParameters::SetInt8},
    {"LoraWAN - DataRate", ConfigType::LORA_DataRate, sizeof(g_configParams._loraDataRate), &g_configParams._loraDataRate, ConfigurationParameters::SetInt8},
    {"LoraWAN - ADR Enabled", ConfigType::LORA_ADREnabled, sizeof(g_configParams._loraADREnabled), &g_configParams._loraADREnabled, ConfigurationParameters::SetBool},
    {"LoraWAN - Require confirmation message", ConfigType::LORA_RequireConfirmation, sizeof(g_configParams._loraRequireConfirmation), &g_configParams._loraRequireConfirmation, ConfigurationParameters::SetBool},
    {"Motion - 1st interrupt duration", ConfigType::MOTION_1stDuration, sizeof(g_configParams._motion1stDuration), &g_configParams._motion1stDuration, ConfigurationParameters::SetUint8},
    {"Motion - 2nd interrupt duration", ConfigType::MOTION_2ndDuration, sizeof(g_configParams._motion2ndDuration), &g_configParams._motion2ndDuration, ConfigurationParameters::SetUint8},
    {"Motion - 1st interrupt threshold (0 == disabled)", ConfigType::MOTION_1stThreshold, sizeof(g_configParams._motion1stThreshold), &g_configParams._motion1stThreshold, ConfigurationParameters::SetUint8},
    {"Motion - 2nd interrupt threshold (0 == disabled)", ConfigType::MOTION_2ndThreshold, sizeof(g_configParams._motion2ndThreshold), &g_configParams._motion2ndThreshold, ConfigurationParameters::SetUint8},
};

void ConfigurationParameters::SetUint32(const ConfigOption *option, uint8_t *arr)
{
    uint32_t val = 0;
    memcpy(&val, arr, option->sizeOfOption);
    uint32_t *ptr = (uint32_t *)option->value;
    *ptr = __builtin_bswap32(val);
    SERIAL_LOG("Setting '%s' change to %u", option->name, *ptr);
}

void ConfigurationParameters::SetUint16(const ConfigOption *option, uint8_t *arr)
{
    uint16_t val = 0;
    memcpy(&val, arr, option->sizeOfOption);
    uint16_t *ptr = (uint16_t *)option->value;
    *ptr = __builtin_bswap16(val);
    SERIAL_LOG("Setting '%s' change to %u", option->name, *ptr);
}

void ConfigurationParameters::SetUint8(const ConfigOption *option, uint8_t *arr)
{
    uint8_t val = 0;
    memcpy(&val, arr, option->sizeOfOption);
    uint8_t *ptr = (uint8_t *)option->value;
    *ptr = val;
    SERIAL_LOG("Setting '%s' change to %u", option->name, *ptr);

}

void ConfigurationParameters::SetInt8(const ConfigOption *option, uint8_t *arr)
{
    int8_t val = 0;
    memcpy(&val, arr, option->sizeOfOption);
    int8_t *ptr = (int8_t *)option->value;
    *ptr = val;
    SERIAL_LOG("Setting '%s' change to %d", option->name, *ptr);

}

void ConfigurationParameters::SetBool(const ConfigOption *option, uint8_t *arr)
{
    bool *ptr = (bool *)option->value;
    *ptr = false;
    // for boolean, we just expect a value > 0 to be true.
    if (arr[0] > 0)
    {
        *ptr = true;
    }
    SERIAL_LOG("Setting '%s' change to %d", option->name, *ptr);

}

void ConfigurationParameters::SetConfig(uint8_t *arr, uint8_t length)
{
    for (uint8_t i = 0; i < length; i++)
    {
        for (size_t x = 0; x < sizeof(g_configs) / sizeof(ConfigOption); x++)
        {
            const ConfigOption *conf = &g_configs[x];
            if (conf->configType == arr[i])
            {
                conf->setfunc(conf, (arr + i + 1));
                i += conf->sizeOfOption;
                break;
            }
        }
    }
}

#endif