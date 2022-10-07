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
};

static const ConfigOption configs[] = {
    {"Sleep time between GPS fixes (in seconds)", ConfigType::SleepTime, sizeof(g_configParams._sleeptime), &g_configParams._sleeptime},
    {"GPS - Dynamic Model", ConfigType::GPSDynamicModel, sizeof(g_configParams._gnssDynamicModel), &g_configParams._gnssDynamicModel},
    {"GPS - Fix timeout (in seconds)", ConfigType::GPSFixTimeout, sizeof(g_configParams._gnssFixTimeout), &g_configParams._gnssFixTimeout}

};

struct ConfigurationParameters
{
    uint32_t _sleeptime = 1000 * 300;
    uint8_t _gnssDynamicModel = 2;
    uint16_t _gnssFixTimeout = 120000UL;

public:
    uint32_t GetSleepTime() const { return _sleeptime; };
    void SetSleepTime(uint32_t timeInSeconds) { _sleeptime = timeInSeconds * 1000; }

    uint16_t GetGNSSFixTimeout() const { return _gnssFixTimeout; }
    void SetGnSSFixTimeout(uint16_t timeout) { _gnssFixTimeout = timeout; }

    uint8_t GetGNSSDynamicModel() const { return _gnssDynamicModel; }
    void SetGNSSDynamicModel(uint8_t dynModel) { _gnssDynamicModel = dynModel; }

    void StoreSetting(ConfigType type, uint8_t *array)
    {
        for (int i = 0; i < 3; i++)
        {
            if (configs[i].configType == type)
            {
                configs[i].value = *array;
            }
        }
    }

} g_configParams;

#endif