#ifndef CONFIG_H
#define CONFIG_H
#include <Arduino.h>
#include <lorahelper.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>

#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>
using namespace Adafruit_LittleFS_Namespace;

// These config settings can be updated remotely.
enum ConfigType
{
    SleepTime0 = 0x10,
    SleepTime1 = 0x11,
    SleepTime2 = 0x12,
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
    // Sheeptracker 1
    // uint8_t _loraDevEUI[8] = {0xAC, 0x1F, 0x09, 0xFF, 0xFE, 0x08, 0xDD, 0xB1};
    // uint8_t _loraNodeAppKey[16] = {0x66, 0x7b, 0x90, 0x71, 0xa1, 0x72, 0x18, 0xd4, 0xcd, 0xb2, 0x13, 0x04, 0x3f, 0xb2, 0x6b, 0x7c};

    // SheepTracker 2
    // uint8_t _loraDevEUI[8] = {0xAC, 0x1F, 0x09, 0xFF, 0xFE, 0x08, 0xF5, 0x2B};
    // uint8_t _loraNodeAppKey[16] = {0x4b, 0xbb, 0x43, 0xef, 0x4b, 0xc6, 0x46, 0x22, 0x1b, 0x0d, 0xcb, 0xe0, 0x44, 0x54, 0xb6, 0x1a};

    // SheepTracker 3
    uint8_t _loraDevEUI[8] = {0xAC, 0x1F, 0x09, 0xFF, 0xFE, 0x0C, 0xD7, 0x8A};
    uint8_t _loraNodeAppKey[16] = {0xde, 0x84, 0xd1, 0xdc, 0x58, 0x7f, 0xf6, 0x8e, 0xe0, 0xd5, 0x31, 0x40, 0xff, 0x76, 0xb2, 0x89};

    // Config settings
    uint16_t _sleeptime0 = 30; // in seconds
    uint16_t _sleeptime1 = 20;
    uint16_t _sleeptime2 = 10;
    uint16_t _gnssFixTimeout = 30; // in seconds
    uint8_t _gnssDynamicModel = dynModel::DYN_MODEL_PEDESTRIAN;

    uint8_t _motion1stThreshold = 0x02;
    uint8_t _motion2ndThreshold = 0x10;
    uint8_t _motion1stDuration = 0x10;
    uint8_t _motion2ndDuration = 0x10;

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

class ConfigHelper
{

public:
    uint32_t GetSleepTime0InSeconds() { return configvalues._sleeptime0; }
    uint32_t GetSleepTime1InSeconds() { return configvalues._sleeptime1; }
    uint32_t GetSleepTime2InSeconds() { return configvalues._sleeptime2; }
    uint16_t GetGNSSFixTimeoutInSeconds() { return configvalues._gnssFixTimeout; }
    // TODO: make this return `dynModel`. Requires a new Setmethod
    uint8_t GetGNSSDynamicModel() { return configvalues._gnssDynamicModel; }

    uint8_t GetLoraTXPower() { return configvalues._loraTXPower; }
    uint8_t GetLoraDataRate() { return configvalues._loraDataRate; }
    bool GetLoraADREnabled() { return configvalues._loraADREnabled; }
    uint8_t *GetLoraDevEUI() { return configvalues._loraDevEUI; }
    uint8_t *GetLoraNodeAppEUI() { return configvalues._loraNodeAppEUI; }
    uint8_t *GetLoraAppKey() { return configvalues._loraNodeAppKey; }
    bool GetLoraRequireConfirmation() { return configvalues._loraRequireConfirmation; }

    uint8_t GetMotion1stDuration() { return configvalues._motion1stDuration; }
    uint8_t GetMotion2ndDuration() { return configvalues._motion2ndDuration; }

    uint8_t GetMotion1stThreshold() { return configvalues._motion1stThreshold; }
    uint8_t GetMotion2ndThreshold() { return configvalues._motion2ndThreshold; }
    void SetConfig(uint8_t *arr, uint8_t length)
    {
        if (length > 0)
        {
            if (arr[0] == ConfigType::ClearConfig)
            {
            }
            else
            {
                for (uint8_t i = 0; i < length; i++)
                {
                    for (size_t x = 0; x < sizeof(configs) / sizeof(ConfigOption); x++)
                    {
                        const ConfigOption *conf = &configs[x];
                        if (conf->configType == arr[i])
                        {
                            conf->setfunc(conf, (arr + i + 1));
                            i += conf->sizeOfOption;
                            break;
                        }
                    }
                }
            }
        }
    }
    bool SaveConfig()
    {
        SERIAL_LOG("Saveconfig called. Storing stuff.");

        File lora_file(InternalFS);
        lora_file.open(CONFIGNAME, FILE_O_READ);
        if (!lora_file)
        {
            SERIAL_LOG("failed to open file for reading");
        }
        lora_file.close();

        if (InternalFS.remove(CONFIGNAME))
        {
            SERIAL_LOG("remove() returned TRUE");
        }
        else
        {
            SERIAL_LOG("remove() returned FALSE");
        }

        if (lora_file.open(CONFIGNAME, FILE_O_WRITE))
        {
            SERIAL_LOG("Open file_O_write returned TRUE");
            lora_file.write((uint8_t *)&configvalues, sizeof(ConfigurationParameters));
            SERIAL_LOG("WRiting config");
            lora_file.flush();
            SERIAL_LOG("FLUSH");
            lora_file.close();
            SERIAL_LOG("Close");
        }
        else
        {
            SERIAL_LOG("Open file_O_write returned FALSE");
            return false;
        }
        return true;
    }

    bool InitConfig()
    {
        InternalFS.begin();
        SERIAL_LOG("Initializing config from flash");
        if (!InternalFS.exists(CONFIGNAME))
        {
            SERIAL_LOG("No Configuration exists. Saving current.")
            SaveConfig();
        }

        File lora_file(InternalFS);

        lora_file.open(CONFIGNAME, FILE_O_READ);
        if (!lora_file)
        {
            SERIAL_LOG("Config initialization done, but still not readable. Flash broken?");
            return false;
        }
        SERIAL_LOG("LOADING CONFIG");
        lora_file.read((uint8_t *)&configvalues, sizeof(ConfigurationParameters));
        lora_file.close();
        return true;
    }

private:
    ConfigurationParameters configvalues;
    inline static const char CONFIGNAME[] = "config.bin";

    ConfigOption configs[13] = {
        {"Sleep time between GPS fixes (in seconds) - no threshold", ConfigType::SleepTime0, sizeof(ConfigurationParameters::_sleeptime0), &configvalues._sleeptime0, ConfigurationParameters::SetUint16},
        {"Sleep time between GPS fixes (in seconds) - 1st threshold", ConfigType::SleepTime1, sizeof(ConfigurationParameters::_sleeptime1), &configvalues._sleeptime1, ConfigurationParameters::SetUint16},
        {"Sleep time between GPS fixes (in seconds) - 2nd threshold", ConfigType::SleepTime2, sizeof(ConfigurationParameters::_sleeptime2), &configvalues._sleeptime2, ConfigurationParameters::SetUint16},
        {"GPS - Fix timeout (in seconds)", ConfigType::GPSFixTimeout, sizeof(ConfigurationParameters::_gnssFixTimeout), &configvalues._gnssFixTimeout, ConfigurationParameters::SetUint16},
        {"GPS - Dynamic Model", ConfigType::GPSDynamicModel, sizeof(ConfigurationParameters::_gnssDynamicModel), &configvalues._gnssDynamicModel, ConfigurationParameters::SetUint8},
        {"LoraWAN - TX Power", ConfigType::LORA_TXPower, sizeof(ConfigurationParameters::_loraTXPower), &configvalues._loraTXPower, ConfigurationParameters::SetInt8},
        {"LoraWAN - DataRate", ConfigType::LORA_DataRate, sizeof(ConfigurationParameters::_loraDataRate), &configvalues._loraDataRate, ConfigurationParameters::SetInt8},
        {"LoraWAN - ADR Enabled", ConfigType::LORA_ADREnabled, sizeof(ConfigurationParameters::_loraADREnabled), &configvalues._loraADREnabled, ConfigurationParameters::SetBool},
        {"LoraWAN - Require confirmation message", ConfigType::LORA_RequireConfirmation, sizeof(ConfigurationParameters::_loraRequireConfirmation), &configvalues._loraRequireConfirmation, ConfigurationParameters::SetBool},
        {"Motion - 1st interrupt duration", ConfigType::MOTION_1stDuration, sizeof(ConfigurationParameters::_motion1stDuration), &configvalues._motion1stDuration, ConfigurationParameters::SetUint8},
        {"Motion - 2nd interrupt duration", ConfigType::MOTION_2ndDuration, sizeof(ConfigurationParameters::_motion2ndDuration), &configvalues._motion2ndDuration, ConfigurationParameters::SetUint8},
        {"Motion - 1st interrupt threshold (0 == disabled)", ConfigType::MOTION_1stThreshold, sizeof(ConfigurationParameters::_motion1stThreshold), &configvalues._motion1stThreshold, ConfigurationParameters::SetUint8},
        {"Motion - 2nd interrupt threshold (0 == disabled)", ConfigType::MOTION_2ndThreshold, sizeof(ConfigurationParameters::_motion2ndThreshold), &configvalues._motion2ndThreshold, ConfigurationParameters::SetUint8},
    };
};


ConfigHelper g_configParams;

#endif