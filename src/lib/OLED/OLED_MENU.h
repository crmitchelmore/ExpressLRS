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

#pragma once

// Default header files for Express LRS
#include "targets.h"
// OLED specific header files. 
#include <U8g2lib.h>   // Needed for the OLED drivers, this is a arduino package. It is maintained by platformIO


typedef void (*longPresscallback)(uint8_t i);
typedef const char *(*getShowStr)(int i);

#if defined(Regulatory_Domain_ISM_2400)
#define  REGULATORT_DOMAIN  "2.4G"
#endif

#if defined(Regulatory_Domain_FCC_915) || defined(Regulatory_Domain_AU_915)
#define  REGULATORT_DOMAIN  "915M"
#endif

#if defined(Regulatory_Domain_EU_868) || defined(Regulatory_Domain_IN_866)
#define  REGULATORT_DOMAIN  "868M"
#endif

#define LOCKTIME 5000
typedef void (*callBackFuntion)(void);

typedef struct 
{
    uint8_t line;
    uint8_t row;
    getShowStr getStr;
} option_t;

typedef struct 
{
    /* data */
    uint8_t x;
    uint8_t y;
    uint8_t length;
    uint8_t hight;
}box_t;

typedef enum 
{
    INDEX_RATE = 0,
    INDEX_TLM,
    INDEX_POWER,
    INDEX_RGB,
    INDEX_BIND,
    INDEX_WIFIUPDATE,
    OLED_MENU_INDEX_COUNT,
} menu_index_e;

typedef struct 
{
    uint8_t index;      //Index of the parameter that is being modified now【pkt.rate  tlm.radio  power  RGB  Bind  update】
    uint8_t value;      //The value of the parameter that is being modified now
    option_t option[2]; //Hash display for each option on each row，Like this:【Pkt.rate  25hz】
    box_t box;          //Display format
    callBackFuntion  leftShortPressCallBack;
    callBackFuntion  rightShortPressCallBack;
    callBackFuntion  middleShortPressCallBack;
} menuItemShow_t;

class OLED_MENU
{    
private:
    static bool inSetupPage;            //If the value is true, means the oled's setup page is being displayed
    static bool inWifiUpdateMode;       //If the value is true, means enter the wifi update mode
    static uint8_t currentIndex;        //Record which row of the OLED is currently in
    static menuItemShow_t menuItem[];   //OLED menu table
    static uint32_t OledLastUpdatedTime;
    static const char * getRgbString(int rgb);
    static const char * getRateString(int rate);
    static const char * getBindString(int bind);
    static const char * getPowerString(int power);
    static const char * getOptionString(int index);
    static const char * getUpdateiString(int update);
    static const char * getTLMRatioString(int ratio);
    void setCommitString(const uint8_t * commit, char * commitStr);

    static void menuUpdata(const char * CommitStr);
    static void updateScreen(int power ,int rate, int tlm,const char * commitStr);  
    static void nullCallback(void);
    static void bindCallBack(void);
    static void updateCallBack(void);
    static void pktRateDecreaseCallBack(void);     
    static void pktRateIncreaseCallBack(void);
    static void tlmRadioDecreaseCallBack(void);
    static void tlmRadioIncreaseCallBack(void);
    static void powerDecreaseCallBack(void);
    static void powerIncreaseCallBack(void);
    static void RGBDecreaseCallBack(void);
    static void RGBIncreaseCallBack(void);
 
public:
    static void Init(void);
    static void start(void);
    static void displayLockScreenLogo(void);
    static void lockScreenOverTime(void);
    static void middleShortPressCallback(void); 
    static void middleLongPressCallback(void);  
    static void leftShortPressCallback(void);   
    static void leftLongPressCallback(void);    
    static void rightShortPressCallback(void);  
    static void rightLongPressCallback(void);
    static void upShortPressCallback(void);     
    static void upLongPressCallback(void);
    static void downShortPressCallback(void);   
    static void dowmLongPressCallback(void);
};


