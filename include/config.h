#pragma once
#include <Arduino.h>
#include <serialhelper.h>
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
    GPSHDOPLimit = 0x22,
    LORA_TXPower = 0x40,
    LORA_DataRate = 0x41,
    LORA_ADREnabled = 0x42,
    LORA_RequireConfirmation = 0x43,
    LORA_DevEUI = 0x48,
    LORA_NodeAppKey = 0x49,
    MOTION_1stThreshold = 0x51,
    MOTION_2ndThreshold = 0x52,
    MOTION_1stDuration = 0x53,
    MOTION_2ndDuration = 0x54,
    Restart = 0xF0,
    ClearConfig = 0xF1,
    SaveConfig = 0xF2
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
    // Sheeptracker 1
    // uint8_t _loraDevEUI[8] = {0xAC, 0x1F, 0x09, 0xFF, 0xFE, 0x08, 0xDD, 0xB1};
    // uint8_t _loraNodeAppKey[16] = {0x66, 0x7b, 0x90, 0x71, 0xa1, 0x72, 0x18, 0xd4, 0xcd, 0xb2, 0x13, 0x04, 0x3f, 0xb2, 0x6b, 0x7c};

    // SheepTracker 2
    // uint8_t _loraDevEUI[8] = {0xAC, 0x1F, 0x09, 0xFF, 0xFE, 0x08, 0xF5, 0x2B};
    // uint8_t _loraNodeAppKey[16] = {0x4b, 0xbb, 0x43, 0xef, 0x4b, 0xc6, 0x46, 0x22, 0x1b, 0x0d, 0xcb, 0xe0, 0x44, 0x54, 0xb6, 0x1a};

    // SheepTracker 3
    // uint8_t _loraDevEUI[8] = {0xAC, 0x1F, 0x09, 0xFF, 0xFE, 0x0C, 0xD7, 0x8A};
    // uint8_t _loraNodeAppKey[16] = {0xde, 0x84, 0xd1, 0xdc, 0x58, 0x7f, 0xf6, 0x8e, 0xe0, 0xd5, 0x31, 0x40, 0xff, 0x76, 0xb2, 0x89};

    // SheepTracker NEW
    uint8_t _loraDevEUI[8] = {0xAC, 0x1F, 0x09, 0xFF, 0xAA, 0xAA, 0xAA, 0xAA};
    uint8_t _loraNodeAppKey[16] = {0x66, 0x7b, 0x90, 0x71, 0xa1, 0x72, 0x18, 0xd4, 0xcd, 0xb2, 0x13, 0x04, 0x3f, 0xAA, 0xAA, 0xAA};

    // Config settings
    uint16_t _sleeptime0 = 300; // in seconds
    uint16_t _sleeptime1 = 120;
    uint16_t _sleeptime2 = 30;
    uint16_t _gnssFixTimeout = 120; // in seconds
    uint8_t _gnssDynamicModel = dynModel::DYN_MODEL_PEDESTRIAN;
    uint16_t _gnssHDOPLimit = 600;

    uint8_t _motion1stThreshold = 0x00;
    uint8_t _motion2ndThreshold = 0x00;
    uint8_t _motion1stDuration = 0x00;
    uint8_t _motion2ndDuration = 0x00;
    void *_dummy;

    int8_t _loraDataRate = DR_2;
    int8_t _loraTXPower = TX_POWER_2;
    bool _loraADREnabled = false;
    bool _loraRequireConfirmation = false;
    uint8_t _loraNodeAppEUI[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    static void SetUint8x8(const ConfigOption *option, uint8_t *arr);
    static void SetUint8x16(const ConfigOption *option, uint8_t *arr);
    static void SetUint32(const ConfigOption *option, uint8_t *arr);
    static void SetUint16(const ConfigOption *option, uint8_t *arr);
    static void SetUint8(const ConfigOption *option, uint8_t *arr);
    static void SetInt8(const ConfigOption *option, uint8_t *arr);
    static void SetBool(const ConfigOption *option, uint8_t *arr);
    static void Restart(const ConfigOption *option, uint8_t *arr);
    static void DoNothing(const ConfigOption *option, uint8_t *arr);
};

class ConfigHelper
{

public:
    uint32_t GetSleepTime0InSeconds() { return configvalues._sleeptime0; }
    uint32_t GetSleepTime1InSeconds() { return configvalues._sleeptime1; }
    uint32_t GetSleepTime2InSeconds() { return configvalues._sleeptime2; }
    uint16_t GetGNSSFixTimeoutInSeconds() { return configvalues._gnssFixTimeout; }
    // TODO: make this return `dynModel`. Requires a new Setmethod
    uint8_t GetGNSSDynamicModel() { return configvalues._gnssDynamicModel; }
    uint16_t GetGNSSHDOPLimit() { return configvalues._gnssHDOPLimit; }

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

    ConfigOption *GetConfigs(size_t *size);
    void SetConfig(uint8_t *arr, uint8_t length);
    bool InitConfig();

private:
    /*
        We've though quite a bit about versioning, as the config is just the ConfigurationParameter struct writen to a file.
        Now, we could introduce versioning on the ConfigType and ConfigurationParameter so that we can write upgrade code.
        We could read the old file, load it and copy the values to the new configuration option and save that (into a new filename).
        This is all cool and would mean we can 'upgrade' without having to reset parameters.
        In reality, we can only upgrade the firmware when the device is connected to USB, and thus the re-programming becomes a fairly easy.
        In short: we will never really write 'upgrade code' to move from V1 to V2.  It is however practical to have the filename different,
        as we can then change that to ignore the 'old' settings.
        */
    const char *CONFIG_NAME = "config_v4.bin";
    const char *OLD_CONFIG_NAMES[3] = {
        "config_v1.bin",
        "config_v2.bin",
        "config_v3.bin"};
    bool SaveConfig();
    void ResetConfig();

    ConfigurationParameters configvalues;

    ConfigOption configs[19] = {
        {"Sleep time between GPS fixes (in seconds) - no threshold", ConfigType::SleepTime0, sizeof(ConfigurationParameters::_sleeptime0), &configvalues._sleeptime0, ConfigurationParameters::SetUint16},
        {"Sleep time between GPS fixes (in seconds) - 1st threshold", ConfigType::SleepTime1, sizeof(ConfigurationParameters::_sleeptime1), &configvalues._sleeptime1, ConfigurationParameters::SetUint16},
        {"Sleep time between GPS fixes (in seconds) - 2nd threshold", ConfigType::SleepTime2, sizeof(ConfigurationParameters::_sleeptime2), &configvalues._sleeptime2, ConfigurationParameters::SetUint16},
        {"GPS - Fix timeout (in seconds)", ConfigType::GPSFixTimeout, sizeof(ConfigurationParameters::_gnssFixTimeout), &configvalues._gnssFixTimeout, ConfigurationParameters::SetUint16},
        {"GPS - Dynamic Model", ConfigType::GPSDynamicModel, sizeof(ConfigurationParameters::_gnssDynamicModel), &configvalues._gnssDynamicModel, ConfigurationParameters::SetUint8},
        {"GPS - Position Accuracy", ConfigType::GPSHDOPLimit, sizeof(ConfigurationParameters::_gnssHDOPLimit), &configvalues._gnssHDOPLimit, ConfigurationParameters::SetUint16},        
        {"LoraWAN - TX Power", ConfigType::LORA_TXPower, sizeof(ConfigurationParameters::_loraTXPower), &configvalues._loraTXPower, ConfigurationParameters::SetInt8},
        {"LoraWAN - DataRate", ConfigType::LORA_DataRate, sizeof(ConfigurationParameters::_loraDataRate), &configvalues._loraDataRate, ConfigurationParameters::SetInt8},
        {"LoraWAN - ADR Enabled", ConfigType::LORA_ADREnabled, sizeof(ConfigurationParameters::_loraADREnabled), &configvalues._loraADREnabled, ConfigurationParameters::SetBool},
        {"LoraWAN - Require confirmation message", ConfigType::LORA_RequireConfirmation, sizeof(ConfigurationParameters::_loraRequireConfirmation), &configvalues._loraRequireConfirmation, ConfigurationParameters::SetBool},
        {"LoraWAN - Dev EUI", ConfigType::LORA_DevEUI, sizeof(ConfigurationParameters::_loraDevEUI), &configvalues._loraDevEUI, ConfigurationParameters::SetUint8x8},
        {"LoraWAN - Node App key", ConfigType::LORA_NodeAppKey, sizeof(ConfigurationParameters::_loraNodeAppKey), &configvalues._loraNodeAppKey, ConfigurationParameters::SetUint8x16},
        {"Motion - 1st interrupt duration", ConfigType::MOTION_1stDuration, sizeof(ConfigurationParameters::_motion1stDuration), &configvalues._motion1stDuration, ConfigurationParameters::SetUint8},
        {"Motion - 2nd interrupt duration", ConfigType::MOTION_2ndDuration, sizeof(ConfigurationParameters::_motion2ndDuration), &configvalues._motion2ndDuration, ConfigurationParameters::SetUint8},
        {"Motion - 1st interrupt threshold (0 == disabled)", ConfigType::MOTION_1stThreshold, sizeof(ConfigurationParameters::_motion1stThreshold), &configvalues._motion1stThreshold, ConfigurationParameters::SetUint8},
        {"Motion - 2nd interrupt threshold (0 == disabled)", ConfigType::MOTION_2ndThreshold, sizeof(ConfigurationParameters::_motion2ndThreshold), &configvalues._motion2ndThreshold, ConfigurationParameters::SetUint8},
        {"Restart Device", ConfigType::Restart, sizeof(ConfigurationParameters::_dummy), &configvalues._dummy, ConfigurationParameters::Restart},
        {"Clear Config", ConfigType::ClearConfig, sizeof(ConfigurationParameters::_dummy), &configvalues._dummy, ConfigurationParameters::DoNothing},
        {"Save Config", ConfigType::SaveConfig, sizeof(ConfigurationParameters::_dummy), &configvalues._dummy, ConfigurationParameters::DoNothing}};
};
