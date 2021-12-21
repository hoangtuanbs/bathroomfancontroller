#ifndef _BATHROOM_EEPROM_
#define _BATHROOM_EEPROM_

#include <ESP_EEPROM.h>

struct ControllerData
{
    int MaxHumidity = 60;
    int MinTemperature = 20;
    int MaxTemperature = 28;
    int FanOnInterval = 1; // in minute
    int FanOffInterval = 10; // in minute
    bool AutomaticFan = true;
} eepRom;

bool saveRomData()
{
    EEPROM.put(0, eepRom);

    return EEPROM.commit();
}

bool loadRomData()
{
    EEPROM.begin(sizeof(ControllerData));

    if (EEPROM.percentUsed()>=0)
    {
        EEPROM.get(0, eepRom);

        return true;
    } else
    {
        return saveRomData();
    }
}

#endif
