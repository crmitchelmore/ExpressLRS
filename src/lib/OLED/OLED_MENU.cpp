/*
 * This file is part of the ExpressLRS distribution (https://github.com/ExpressLRS/ExpressLRS).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#if defined(USE_OLED_SPI) || defined(USE_OLED_SPI_SMALL) || defined(USE_OLED_I2C)

#include "targets.h"
#include "OLED_MENU.h"
#include "string.h"
#include "XBMStrings.h" // Contains all the express logos and animation for UI
#include "common.h"
#include "POWERMGNT.h"
#include "config.h"

extern TxConfig config;
bool OLED_MENU::inSetupPage = false;
bool OLED_MENU::inWifiUpdateMode = false;
uint8_t  OLED_MENU::currentIndex = 0;
uint32_t OLED_MENU::OledLastUpdatedTime = 0;
const char thisCommit[] = {LATEST_COMMIT, 0};
extern void OLED_MENU_start(void);
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R2, GPIO_PIN_OLED_RST, GPIO_PIN_OLED_SCK, GPIO_PIN_OLED_SDA);

menuItemShow_t OLED_MENU::menuItem[] ={
    {
        INDEX_RATE,//Index of the menu's first option (Pkt.rate in here）
        0,//Default value of the menu's first option
        {
            {0,  20,  getOptionString}, //Display string of the option
            {80, 20,  getRateString}    //Display string of the option's value
        },
        {79,12,31,9},                   //Display format
        pktRateDecreaseCallBack,        //Callback function of press the 5D button left
        pktRateIncreaseCallBack,        //Callback function of press the 5D button right
        nullCallback                    //Callback function of press the 5D button middle
    },
    {
        INDEX_TLM,//Index of the menu's second option (TLM Ratio in here）
        0,
        {
            {0,  30,  getOptionString},
            {80, 30,  getTLMRatioString}
        },
        {79,21,31,10},
        tlmRadioDecreaseCallBack,
        tlmRadioIncreaseCallBack,
        nullCallback
    },
    {
        INDEX_POWER,
        0,
        {
            {0,  40,  getOptionString},
            {80, 40,  getPowerString}
        },
        {79,32,30,10},
        powerDecreaseCallBack,
        powerIncreaseCallBack,
        nullCallback
    },
    {
        INDEX_RGB,
        0,
        {
            {0,  50,  getOptionString},
            {80, 50,  getRgbString}
        },
        {79,42,30,10},
        RGBDecreaseCallBack,
        RGBIncreaseCallBack,
        nullCallback
    },
    {
        INDEX_BIND,
        0,
        {
            {0, 62,  getOptionString},
            {0, 62,  getBindString}
        },
        {0,53,30,11},
        nullCallback,
        nullCallback,
        bindCallBack
    },
    {
        INDEX_WIFIUPDATE,
        0,
        {
            {80, 62,  getOptionString},
            {80, 62,  getUpdateiString}
        },
        {79,53,30,11},
        nullCallback,
        nullCallback,
        updateCallBack
    },
};


void OLED_MENU::Init(void)
{
     u8g2.begin();
}

void OLED_MENU::displayLockScreenLogo()
{
    u8g2.clearBuffer();
    u8g2.drawXBM(0, 0, 64, 64, elrs64);
    u8g2.setFont(u8g2_font_HelvetiPixelOutline_tr);
    u8g2.drawUTF8(76,30, "ELRS");
    u8g2.drawUTF8(78,50,REGULATORT_DOMAIN);
    u8g2.sendBuffer();
    OLED_MENU::inSetupPage = false;
}

void OLED_MENU::lockScreenOverTime(void)
{
    uint32_t now = millis();
    //The oled screen is automatically locked(display the ELRS logo) if there is no any operation in the setup page during timeout
    if((now > OledLastUpdatedTime + LOCKTIME) && (true == OLED_MENU::inSetupPage) &&(false == OLED_MENU::inWifiUpdateMode))
    {
        displayLockScreenLogo();
        config.SetRate(menuItem[INDEX_RATE].value);
        config.SetTlm(menuItem[INDEX_TLM].value);
        config.SetPower(menuItem[INDEX_POWER].value);
    }
    //If enter wifi update mode automatically due to no CRSF ever detected,OLED has also display the 'WIFI Update' page
    if(connectionState == wifiUpdate && (false == OLED_MENU::inWifiUpdateMode))
    {
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_HelvetiPixelOutline_tr);
        u8g2.drawUTF8(15,20, "WIFI Update");
        u8g2.setFont(u8g2_font_6x12_tf);
        u8g2.drawUTF8(14,40, "http://10.0.0.1/");
        u8g2.drawUTF8(20,53, "PW: expresslrs");
        u8g2.sendBuffer();
        OLED_MENU::inWifiUpdateMode = true;
    }

}

void OLED_MENU::menuUpdata(const char * CommitStr)
{
    u8g2.clearBuffer();
    u8g2.setFontMode(1);  /* activate transparent font mode */
    u8g2.setDrawColor(2); /* color 1 for the box */
    u8g2.drawBox(0,0,128, 10);
    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.drawUTF8(90,8,CommitStr);
    u8g2.drawUTF8(3,8, "ExpressLRS");

    for(int i=0;i<6;i++)
    {
        u8g2.drawUTF8(menuItem[i].option[0].line,menuItem[i].option[0].row,menuItem[i].option[0].getStr(menuItem[i].index));

        if(currentIndex == menuItem[i].index)
        {
            u8g2.drawBox(menuItem[i].box.x,menuItem[i].box.y,strlen(menuItem[i].option[1].getStr(menuItem[i].value))*6+1,menuItem[i].box.hight);
        }

        u8g2.setDrawColor(2);
        u8g2.drawUTF8(menuItem[i].option[1].line,menuItem[i].option[1].row,menuItem[i].option[1].getStr(menuItem[i].value));
    }
    u8g2.sendBuffer();
    OLED_MENU::OledLastUpdatedTime = millis();
}

void OLED_MENU::updateScreen(int power ,int rate, int tlm,const char * commitStr)
{
    switch (rate)
    {
#if defined(Regulatory_Domain_ISM_2400)
    //0:"500HZ"   1: "250HZ"  2:"150HZ"  3:"50HZ"
    case RATE_500HZ: rate = 0;break;
    case RATE_250HZ: rate = 1;break;
    case RATE_150HZ: rate = 2;break;
    case RATE_50HZ: rate = 3;break;
#endif

#if defined(Regulatory_Domain_AU_915) || defined(Regulatory_Domain_EU_868)  || defined(Regulatory_Domain_IN_866) || defined(Regulatory_Domain_FCC_915) || defined(Regulatory_Domain_AU_433) || defined(Regulatory_Domain_EU_433)
    //0:"200HZ"   1: "100HZ"  2:"50HZ"  3:"25HZ"
    case RATE_200HZ: rate = 0;break;
    case RATE_100HZ: rate = 1;break;
    case RATE_50HZ: rate = 2;break;
    case RATE_25HZ: rate = 3;break;
#endif
     default: rate = 0;
    }
    menuItem[INDEX_RATE].value = rate;
    menuItem[INDEX_TLM].value = tlm;
    menuItem[INDEX_POWER].value = power;
    u8g2.clearBuffer();
    u8g2.setFontMode(1);  /* activate transparent font mode */
    u8g2.setDrawColor(2); /* color 1 for the box */
    u8g2.drawBox(0,0,128, 10);
    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.drawUTF8(90,8,commitStr);
    u8g2.drawUTF8(3,8, "ExpressLRS");
    for(int i=0;i<6;i++)
    {
        u8g2.drawUTF8(menuItem[i].option[0].line,menuItem[i].option[0].row,menuItem[i].option[0].getStr(menuItem[i].index));
        if(currentIndex == menuItem[i].index)
        {
            u8g2.drawBox(menuItem[i].box.x,menuItem[i].box.y,strlen(menuItem[i].option[1].getStr(menuItem[i].value))*6+1,menuItem[i].box.hight);
        }
        u8g2.setDrawColor(2);
        u8g2.drawUTF8(menuItem[i].option[1].line,menuItem[i].option[1].row,menuItem[i].option[1].getStr(menuItem[i].value));
    }
    u8g2.sendBuffer();
    OLED_MENU::OledLastUpdatedTime = millis();
}


void OLED_MENU::pktRateDecreaseCallBack(void)
{
    if (menuItem[INDEX_RATE].value >= 0 && menuItem[INDEX_RATE].value < RATE_MAX-1)
       menuItem[INDEX_RATE].value++;
    else
        menuItem[INDEX_RATE].value = RATE_MAX-1;
}

void OLED_MENU::pktRateIncreaseCallBack(void)
{
    if (menuItem[INDEX_RATE].value > 0 && menuItem[INDEX_RATE].value <= RATE_MAX-1)
       menuItem[INDEX_RATE].value--;
    else
        menuItem[INDEX_RATE].value = 0;
}

void OLED_MENU::tlmRadioDecreaseCallBack(void)
{
    if (menuItem[INDEX_TLM].value > TLM_RATIO_NO_TLM && menuItem[INDEX_TLM].value <= TLM_RATIO_1_2)
        menuItem[INDEX_TLM].value--;
    else
        menuItem[INDEX_TLM].value = TLM_RATIO_NO_TLM;
}
void OLED_MENU::tlmRadioIncreaseCallBack(void)
{
    if (menuItem[INDEX_TLM].value < TLM_RATIO_1_2 && menuItem[INDEX_TLM].value >= TLM_RATIO_NO_TLM)
        menuItem[INDEX_TLM].value++;
    else
        menuItem[INDEX_TLM].value = TLM_RATIO_1_2;
}

void OLED_MENU::powerDecreaseCallBack(void)
{
    if (menuItem[INDEX_POWER].value > MinPower && menuItem[INDEX_POWER].value <= MaxPower)
       menuItem[INDEX_POWER].value--;
    else
        menuItem[INDEX_POWER].value = MinPower;
}

void OLED_MENU::powerIncreaseCallBack(void)
{
    if (menuItem[INDEX_POWER].value >= MinPower && menuItem[INDEX_POWER].value < MaxPower)
       menuItem[INDEX_POWER].value++;
    else
        menuItem[INDEX_POWER].value = MaxPower;
}

void OLED_MENU::RGBDecreaseCallBack(void)
{
    //RGB is automatically changes according to the current system state
}

void OLED_MENU::RGBIncreaseCallBack(void)
{
    //RGB is automatically changes according to the current system state
}
void EnterBindingMode();
void OLED_MENU::bindCallBack(void)
{
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_unifont_t_symbols);
    u8g2.drawGlyph(58, 30, 0x23f3);	/* dec 9731/hex 2603 Snowman */
    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.drawUTF8(38,50, "Binding...");
    u8g2.sendBuffer();
    delay(500);
    EnterBindingMode();
}

void OLED_MENU::updateCallBack(void)
{
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_HelvetiPixelOutline_tr);
    u8g2.drawUTF8(15,20, "WIFI Update");
    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.drawUTF8(14,40, "http://10.0.0.1/");
    u8g2.drawUTF8(20,53, "PW: expresslrs");
    u8g2.sendBuffer();
    if (connectionState != wifiUpdate)
    {
        connectionState = wifiUpdate;
        OLED_MENU::inWifiUpdateMode = true;
    }
}
//entering or exiting the OLED setup page
void OLED_MENU::middleLongPressCallback(void)
{
    if(false == OLED_MENU::inSetupPage)
    {
        OLED_MENU::updateScreen(POWERMGNT::currPower(),ExpressLRS_currAirRate_Modparams->enum_rate,ExpressLRS_currAirRate_Modparams->TLMinterval,thisCommit);
        OLED_MENU::inSetupPage = true;
    }
    else
    {
        displayLockScreenLogo();
        config.SetRate(menuItem[INDEX_RATE].value);
        config.SetTlm(menuItem[INDEX_TLM].value);
        config.SetPower(menuItem[INDEX_POWER].value);
        OLED_MENU::inSetupPage = false;
    }
}

//Confirm button[for entering binding mode or wifiupdate mode]
void OLED_MENU::middleShortPressCallback(void)
{
     if(true == OLED_MENU::inSetupPage)
    {
        menuItem[currentIndex].middleShortPressCallBack();
        if(false == OLED_MENU::inWifiUpdateMode)
            OLED_MENU::menuUpdata(thisCommit);
    }
}
//Switch options upward
void OLED_MENU::upShortPressCallback(void)
{
    if(true == OLED_MENU::inSetupPage)
    {        
        currentIndex --;
        if(currentIndex < 0 || currentIndex >= OLED_MENU_INDEX_COUNT)
        {
            currentIndex = OLED_MENU_INDEX_COUNT-1;
        }
        OLED_MENU::menuUpdata(thisCommit);
    }
}
//Switch options downward
void OLED_MENU::downShortPressCallback(void)
{
    if(true == OLED_MENU::inSetupPage)
    {
        currentIndex ++;
        if(currentIndex < 0 || currentIndex >= OLED_MENU_INDEX_COUNT)
        {
            currentIndex = 0;
        }
        OLED_MENU::menuUpdata(thisCommit);
    }
}
//Decreasing the selected parameter
void OLED_MENU::leftShortPressCallback(void)
{
    if(true == OLED_MENU::inSetupPage)
    {
        menuItem[currentIndex].leftShortPressCallBack();
        OLED_MENU::menuUpdata(thisCommit);
    }

}
//Increasing the selected parameter
void OLED_MENU::rightShortPressCallback(void)
{
    if(true == OLED_MENU::inSetupPage)
    {
        menuItem[currentIndex].rightShortPressCallBack();
        OLED_MENU::menuUpdata(thisCommit);
    }
}

void OLED_MENU::nullCallback(void) {}


const char *OLED_MENU::getOptionString(int index)
{
    switch(index)
    {
        case 0: return "Pkt.rate";
        case 1: return "TLM Ratio";
        case 2: return "Power";
        case 3: return "RGB";
        case 4: return "";
        case 5: return "";
        default:return "Error";
    }
}

const char * OLED_MENU::getBindString(int bind)
{
    return "[Bind]";
}


const char * OLED_MENU::getUpdateiString(int update)
{
    return "[Update]";
}


const char * OLED_MENU::getRgbString(int rgb)
{
      return "auto";
}

/**
 * Returns power level string (Char array)
 *
 * @param values power = int/enum for current power level.
 * @return const char array for power level Ex: "500 mW\0"
 */
const char *OLED_MENU::getPowerString(int power){
    switch (power)
    {
    case 0: return "10mW";
    case 1: return "25mW";
    case 2: return "50mW";
    case 3: return "100mW";
    case 4: return "250mW";
    case 5: return "500mW";
    case 6: return "1000mW";
    case 7: return "2000mW";
    default: return "Error";
    }
}

/**
 * Returns packet rate string (Char array)
 *
 * @param values rate = int/enum for current packet rate.
 * @return const char array for packet rate Ex: "500 hz\0"
 */
const char * OLED_MENU::getRateString(int rate){
    switch (rate)
    {
#if defined(Regulatory_Domain_ISM_2400)
    case 0: return "500hz";
    case 1: return "250hz";
    case 2: return "150hz";
    case 3: return "50hz";
#endif

#if defined(Regulatory_Domain_AU_915) || defined(Regulatory_Domain_EU_868)  || defined(Regulatory_Domain_IN_866) || defined(Regulatory_Domain_FCC_915) || defined(Regulatory_Domain_AU_433) || defined(Regulatory_Domain_EU_433)
      case 0: return "200hz";
      case 1: return "100hz";
      case 2: return "50hz";
      case 3: return "25hz";
#endif
     default: return "ERR";
    }
}

/**
 * Returns telemetry ratio string (Char array)
 *
 * @param values ratio = int/enum for current power level.
 * @return const char array for telemetry ratio Ex: "1:128\0"
 */
const char *OLED_MENU::getTLMRatioString(int ratio){
    switch (ratio)
    {
    case 0: return "OFF";
    case 1: return "1:128";
    case 2: return "1:64";
    case 3: return "1:32";
    case 4: return "1:16";
    case 5: return "1:8";
    case 6: return "1:4";
    case 7: return "1:2";
    default: return "ERR";
    }
}

/**
 * Sets the commit string by converting unsigned integers to signed char's (Char array)
 *
 * @param values commit = unsigned 8 byte int array for the commit and char array for commit string.
 * @return void
 */
void OLED_MENU::setCommitString(const uint8_t * commit, char * commitStr){

    for(int i = 0; i < 6; i++ ){
        if (commit[i] < 10) commitStr[i] = commit[i] + 48; // gets us 0 to 9
        else commitStr[i] = commit[i] + 87; // gets us a to f
    }

}
#endif
