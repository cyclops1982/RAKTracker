#include <stdio.h>
#include <stdint.h>
#include <cstdlib>
#include <cstring>
#include <arpa/inet.h>

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
    void (*setfunc)(ConfigOption opt, uint8_t *arr);
};

struct ConfigurationParameters
{
    uint32_t _sleeptime = 300000;
    uint8_t _gnssDynamicModel = 2;
    uint16_t _gnssFixTimeout = 100;

public:
    uint32_t GetSleepTime() const { return _sleeptime; };
    // void SetSleepTime(uint32_t timeInSeconds) { _sleeptime = timeInSeconds; }

    uint16_t GetGNSSFixTimeout() const { return _gnssFixTimeout; }
    // void SetGnSSFixTimeout(uint16_t timeout) { _gnssFixTimeout = timeout; }

    uint8_t GetGNSSDynamicModel() const { return _gnssDynamicModel; }
    // void SetGNSSDynamicModel(uint8_t dynModel) { _gnssDynamicModel = dynModel; }

    void PrintAll();

    static void SetUint32(ConfigOption option, uint8_t *arr);
    static void SetUint16(ConfigOption option, uint8_t *arr);

} g_configParams;

void ConfigurationParameters::SetUint32(ConfigOption option, uint8_t *arr)
{
    uint32_t val = 0;
    memcpy(&val, arr, option.sizeOfOption);
    uint32_t *ptr = (uint32_t *)option.value;
    *ptr = htonl(val);
}

void ConfigurationParameters::SetUint16(ConfigOption option, uint8_t *arr)
{
    uint16_t val = 0;
    memcpy(&val, arr, option.sizeOfOption);
    uint16_t *ptr = (uint16_t *)option.value;
    *ptr = htons(val);
}

static const ConfigOption configs[] = {
    {"Sleep time between GPS fixes (in seconds)", ConfigType::SleepTime, sizeof(g_configParams._sleeptime), &g_configParams._sleeptime, ConfigurationParameters::SetUint32},
    {"GPS - Dynamic Model", ConfigType::GPSDynamicModel, sizeof(g_configParams._gnssDynamicModel), &g_configParams._gnssDynamicModel, ConfigurationParameters::SetUint32},
    {"GPS - Fix timeout (in seconds)", ConfigType::GPSFixTimeout, sizeof(g_configParams._gnssFixTimeout), &g_configParams._gnssFixTimeout, ConfigurationParameters::SetUint16}

};

void ConfigurationParameters::PrintAll()
{

    printf("==================================\n");
    for (int i = 0; i < 3; i++)
    {

        printf("Config type: %d\n", configs[i].configType);
        printf("Config Name: %s\n", configs[i].name);
        switch (configs[i].configType)
        {
        case ConfigType::SleepTime:

            printf("Config Value %u\n", g_configParams.GetSleepTime());
            break;
        case ConfigType::GPSFixTimeout:
            printf("Config VALUE %d\n", g_configParams.GetGNSSFixTimeout());
            break;

        default:
            break;
        }
        printf("---\n");
    }
    printf("==================================\n");
}

int main()
{
    uint8_t input[8];
    input[0] = 0x21;
    input[1] = 0x01;
    input[2] = 0xFF;
    input[3] = 0x01;
    input[4] = 0x00;
    input[5] = 0x00;
    input[6] = 0x01;
    input[7] = 0xFF;

    g_configParams.PrintAll();
    for (int x = 0; x < sizeof(input); x++)
    {
        printf("X = %d\n", x);
        for (int i = 0; i < 3; i++)
        {
            if (configs[i].configType == input[x])
            {
                printf("ConfigType match on type 0x%02x - size: %lu\n", configs[i].configType, configs[i].sizeOfOption);
                configs[i].setfunc(configs[i], (input + x + 1));
                x += configs[i].sizeOfOption;
                break;
            }
        }
    }
    g_configParams.PrintAll();

    return 0;
}
