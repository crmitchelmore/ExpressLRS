#include "targets.h"
#include "common.h"
#include "device.h"

#if defined(USE_OLED_SPI) || defined(USE_OLED_SPI_SMALL) || defined(USE_OLED_I2C)

#include "POWERMGNT.h"
#if defined(TARGET_TX_BETAFPV_2400_MICRO_V1) || defined(TARGET_TX_BETAFPV_900_MICRO_V1)
#include "OLED_MENU.h"    
#else
#include "OLED.h"
#endif
static int start()
{
#if defined(TARGET_TX_BETAFPV_2400_MICRO_V1) || defined(TARGET_TX_BETAFPV_900_MICRO_V1)
    OLED_MENU::Init();
    OLED_MENU::displayLockScreenLogo();
return 1000;
#else    
    OLED::displayLogo();
    return 1000;    // set callback in 1s
#endif
}

static int timeout()
{
#if defined(TARGET_TX_BETAFPV_2400_MICRO_V1) || defined(TARGET_TX_BETAFPV_900_MICRO_V1)

    OLED_MENU::lockScreenOverTime();
    return 300; // check for updates every 300ms
#else  

    static PowerLevels_e lastPower;
    static expresslrs_RFrates_e lastRate;
    static expresslrs_tlm_ratio_e lastRatio;
    const char thisCommit[] = {LATEST_COMMIT, 0};

    PowerLevels_e newPower = POWERMGNT::currPower();
    expresslrs_RFrates_e newRate = ExpressLRS_currAirRate_Modparams->enum_rate;
    expresslrs_tlm_ratio_e newRatio = ExpressLRS_currAirRate_Modparams->TLMinterval;

    if (lastPower != newPower || lastRate != newRate || lastRatio != newRatio)
    {
        OLED::updateScreen(newPower, newRate, newRatio, thisCommit);
        lastPower = newPower;
        lastRate = newRate;
        lastRatio = newRatio;
    }

    return 300; // check for updates every 300ms
#endif
}

device_t OLED_device = {
    .initialize = nullptr,
    .start = start,
    .event = nullptr,
    .timeout = timeout
};

#else

device_t OLED_device = {
    NULL
};

#endif
