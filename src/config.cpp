#include "config.h"

void ConfigurationParameters::Restart(const ConfigOption *option, uint8_t *arr)
{
    SERIAL_LOG("Restarting...");
    NVIC_SystemReset();
}
void ConfigurationParameters::DoNothing(const ConfigOption *option, uint8_t *arr)
{
    SERIAL_LOG("Doing nothing");
}

void ConfigurationParameters::SetUint8x16(const ConfigOption *option, uint8_t *arr)
{
    uint8_t *ptr = (uint8_t *)option->value;
    memcpy(ptr, arr, option->sizeOfOption);
    SERIAL_LOG("Setting '%s' change to  0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X", option->name, ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5], ptr[6], ptr[7], ptr[8], ptr[9], ptr[10], ptr[11], ptr[12], ptr[13], ptr[14], ptr[15]);
}

void ConfigurationParameters::SetUint8x8(const ConfigOption *option, uint8_t *arr)
{
    uint8_t *ptr = (uint8_t *)option->value;
    memcpy(ptr, arr, option->sizeOfOption);
    SERIAL_LOG("Setting '%s' change to  0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X", option->name, ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5], ptr[6], ptr[7]);
}

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

ConfigOption *ConfigHelper::GetConfigs(size_t *size)
{
    *size = sizeof(configs) / sizeof(ConfigOption);
    return configs;
};

void ConfigHelper::SetConfig(uint8_t *arr, uint8_t length)
{
    SERIAL_LOG("Setting configuration. Length is: %d", length);
    if (length > 0)
    {
        for (uint8_t i = 0; i < length; i++)
        {
            for (size_t x = 0; x < sizeof(configs) / sizeof(ConfigOption); x++)
            {                
                const ConfigOption *conf = &configs[x];
                if (arr[i] == ConfigType::ClearConfig)
                {
                    ResetConfig();
                    break;
                }
                if (arr[i] == ConfigType::SaveConfig)
                {
                    SaveConfig();
                    break;
                }
                if (conf->configType == arr[i])
                {
                    SERIAL_LOG("Calling setfunc for '%s'", conf->name);
                    conf->setfunc(conf, (arr + i + 1));
                    i += conf->sizeOfOption;
                    break;
                }
            }
        }
    }
}
bool ConfigHelper::SaveConfig()
{
    SERIAL_LOG("Saving configuration file with name %s", CONFIG_NAME);

    File lora_file(InternalFS);

    for (auto file : OLD_CONFIG_NAMES)
    {
        if (InternalFS.exists(file))
        {
            if (InternalFS.remove(file))
            {
                SERIAL_LOG("Removed old config: %s", file);
            }
        }
    }
    if (InternalFS.remove(CONFIG_NAME))
    {
        SERIAL_LOG("remove() returned TRUE");
    }
    else
    {
        SERIAL_LOG("remove() returned FALSE");
    }

    if (lora_file.open(CONFIG_NAME, FILE_O_WRITE))
    {
        lora_file.write((uint8_t *)&configvalues, sizeof(ConfigurationParameters));
        lora_file.flush();
        lora_file.close();
    }
    else
    {
        SERIAL_LOG("Failed to open config file for saving.");
        return false;
    }
    SERIAL_LOG("Save succesful");
    return true;
}

void ConfigHelper::ResetConfig()
{
    SERIAL_LOG("Resetting config to default");
    ConfigurationParameters params;
    configvalues = params;
}

bool ConfigHelper::InitConfig()
{
    InternalFS.begin();
    if (!InternalFS.exists(CONFIG_NAME))
    {
        SERIAL_LOG("No Configuration exists. Saving current.")
        SaveConfig();
    }

    File lora_file(InternalFS);

    lora_file.open(CONFIG_NAME, FILE_O_READ);
    if (!lora_file)
    {
        SERIAL_LOG("Config initialization done, but still not readable. Flash broken?");
        return false;
    }
    lora_file.read((uint8_t *)&configvalues, sizeof(ConfigurationParameters));
    lora_file.close();
    SERIAL_LOG("Loaded configuration from flash")

    // This commented bit of code can be used to override (/hardcode) some of the values. This makes sure that even
    // if you had this in the config, you'd overwrite it to the default value.
    //ConfigurationParameters defaults;
    //memcpy(configvalues._loraDevEUI, defaults._loraDevEUI, 8);
    //memcpy(configvalues._loraNodeAppKey, defaults._loraNodeAppKey, 16);
    //SaveConfig();
    return true;
}