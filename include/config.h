#ifndef CONFIG_H
#define CONFIG_H
#include <Arduino.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>

enum ConfigType
{
    SleepTime = 0x01,
    GPSDynamicModel = 0x20,
    GPSFixTimeout = 0x21
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

    uint32_t _sleeptime = 300000;
    uint16_t _gnssFixTimeout = 100; // in seconds
    uint8_t _gnssDynamicModel = dynModel::DYN_MODEL_BIKE;

    static void SetUint32(const ConfigOption *option, uint8_t *arr);
    static void SetUint16(const ConfigOption *option, uint8_t *arr);
    static void SetUint8(const ConfigOption *option, uint8_t *arr);

public:
    uint32_t GetSleepTime() const { return _sleeptime; };
    uint16_t GetGNSSFixTimeout() const { return _gnssFixTimeout; }
    // TODO: make this return `dynModel`. Requires a new Setmethod
    uint8_t GetGNSSDynamicModel() const { return _gnssDynamicModel; }

    void PrintAll();
    void SetConfig(uint8_t *array, uint8_t length);

} g_configParams;

void ConfigurationParameters::SetUint32(const ConfigOption *option, uint8_t *arr)
{
    uint32_t val = 0;
    memcpy(&val, arr, option->sizeOfOption);
    uint32_t *ptr = (uint32_t *)option->value;
    *ptr = __builtin_bswap32(val);
}

void ConfigurationParameters::SetUint16(const ConfigOption *option, uint8_t *arr)
{
    uint16_t val = 0;
    memcpy(&val, arr, option->sizeOfOption);
    uint16_t *ptr = (uint16_t *)option->value;
    *ptr = __builtin_bswap16(val);
}

void ConfigurationParameters::SetUint8(const ConfigOption *option, uint8_t *arr)
{
    uint8_t val = 0;
    memcpy(&val, arr, option->sizeOfOption);
    uint8_t *ptr = (uint8_t *)option->value;
    *ptr = val;
}

static const ConfigOption g_configs[] = {
    {"Sleep time between GPS fixes (in seconds)", ConfigType::SleepTime, sizeof(g_configParams._sleeptime), &g_configParams._sleeptime, ConfigurationParameters::SetUint32},
    {"GPS - Fix timeout (in seconds)", ConfigType::GPSFixTimeout, sizeof(g_configParams._gnssFixTimeout), &g_configParams._gnssFixTimeout, ConfigurationParameters::SetUint16},
    {"GPS - Dynamic Model", ConfigType::GPSDynamicModel, sizeof(g_configParams._gnssDynamicModel), &g_configParams._gnssDynamicModel, ConfigurationParameters::SetUint8},
};

void ConfigurationParameters::SetConfig(uint8_t *arr, uint8_t length)
{
    for (size_t i; i < length; i++)
    {
        for (size_t x; x < sizeof(g_configs) / sizeof(ConfigOption); x++)
        {
            const ConfigOption *conf = &g_configs[x];
            if (conf->configType == arr[i])
            {
                conf->setfunc(conf, (arr + i + 1));
            }
        }
    }
}

#endif