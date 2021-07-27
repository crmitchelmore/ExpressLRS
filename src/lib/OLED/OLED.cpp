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

#ifdef HAS_OLED // This code will not be used if the hardware does not have a OLED display. Maybe a better way to blacklist it in platformio.ini?

// Default header files for Express LRS
#include "targets.h"
// OLED specific header files
#include "OLED.h"
#include <U8g2lib.h>    // Needed for the OLED drivers, this is a arduino package. It is maintained by platformIO
#include "XBMStrings.h" // Contains all the express logos and animation for UI


#if defined HAS_OLED_128_32
// https://github.com/olikraus/u8g2/wiki/u8g2setupcpp
// https://www.winstar.com.tw/products/oled-module/graphic-oled-display/wearable-displays.html
// Used on the Ghost TX Lite
U8G2_SSD1306_128X32_UNIVISION_F_4W_SW_SPI u8g2(U8G2_R0, GPIO_PIN_OLED_SCK, GPIO_PIN_OLED_MOSI, GPIO_PIN_OLED_CS, GPIO_PIN_OLED_DC, GPIO_PIN_OLED_RST);
#elif defined HAS_I2C_OLED
// https://github.com/olikraus/u8g2/wiki/u8g2setupcpp
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R2, GPIO_PIN_OLED_SCL, GPIO_PIN_OLED_SDA);
#else
#ifdef HAS_OLED_I2C
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, GPIO_PIN_OLED_RST, GPIO_PIN_OLED_SCK, GPIO_PIN_OLED_SDA);
#else 
// https://github.com/olikraus/u8g2/wiki/u8g2setupcpp
U8G2_SSD1306_128X64_NONAME_F_4W_SW_SPI u8g2(U8G2_R0, GPIO_PIN_OLED_SCK, GPIO_PIN_OLED_MOSI, GPIO_PIN_OLED_CS, GPIO_PIN_OLED_DC, GPIO_PIN_OLED_RST);
#endif
#endif

/**
 * helper function is used to draw xbmp on the OLED. 
 * x = x position of the image
 * y = y position of the image
 * size = demensions of the box size x size, this only works for square images 1:1
 * image = XBM character string
 */
void helper(int x, int y, int size,  const unsigned char * image){
    u8g2.clearBuffer();
    u8g2.drawXBMP(x, y, size, size, image);
    u8g2.sendBuffer();
}

/**
 *  ghostChase will only be called for ghost TX hardware.  
 */
void ghostChase(){
    // Using i < 16 and (i*4) to get 64 total pixels. Change to i < 32 (i*2) to slow animation. 
    for(int i = 0; i < 20; i++){
        u8g2.clearBuffer();
        #ifndef TARGET_TX_GHOST_LITE
            u8g2.drawXBMP((26 + i), 16, 32, 32, ghost);
            u8g2.drawXBMP((-31 + (i*4)), 16, 32, 32, elrs32);
        #else
            u8g2.drawXBMP((26 + i), 0, 32, 32, ghost);
            u8g2.drawXBMP((-31 + (i*4)), 0, 32, 32, elrs32);
        #endif
        u8g2.sendBuffer();
    }
    /**
     *  Animation for the ghost logo expanding in the center of the screen.
     *  helper function just draw's the XBM strings.   
     */
    #ifndef TARGET_TX_GHOST_LITE
    helper(38,12,40,elrs40);
    helper(36,8,48,elrs48);
    helper(34,4,56,elrs56);
    helper(32,0,64,elrs64);
    #endif
}


#define PACKED __attribute__((packed))
#define LOCKTIME 5000

enum{
    PKTRATE=0,
    TLMRADIO,
    POWERLEVEL,
    RGBCOLOR,
    BINDING,
    WIFIUPDATE
};


uint32_t lastProcessTime=0;
uint8_t currentIndex = 0;
uint8_t showBaseIndex = 0;
uint8_t screenLocked = 0;

typedef void (*shortPressCallback)(uint8_t i);
typedef void (*longPressCallback)(uint8_t i);
typedef const char *(*getShowStr)(int i);

static const char * getPowerString(int power);
static const char * getRateString(int rate);
static const char * getTLMRatioString(int ratio);
static const char * getRgbString(int rgb);
static const char * getBindString(int bind);
static const char * getUpdateiString(int update);

static void pktRateLPCB(uint8_t i);
static void tlmLPCB(uint8_t i);
static void powerLPCB(uint8_t i);
static void rgbLPCB(uint8_t i);
static void bindLPCB(uint8_t i);
static void updateLPCB(uint8_t i);
typedef struct 
{
    uint8_t line;
    uint8_t value;
    uint8_t index;
    longPressCallback  lpcb;
    getShowStr getStr;
} PACKED screenShow_t;

screenShow_t currentItem[] ={
    {80,0,PKTRATE,pktRateLPCB,getRateString},

    {80,1,TLMRADIO,tlmLPCB,getTLMRatioString},

    {80,2,POWERLEVEL,powerLPCB,getPowerString},

    {80,3,RGBCOLOR,rgbLPCB,getRgbString},

    {80,4,BINDING,bindLPCB,getBindString},

    {80,5,WIFIUPDATE,updateLPCB,getUpdateiString},
};

typedef struct {
    const char *str;
} PACKED showString_t;

showString_t showString[] ={
    "PktRate ",
    "TLM ",
    "Power ",
    "Rgb ",
    "Bind ",
    "Update ",
};


/**
 * Displays the ExpressLRS logo
 *
 * @param values none
 * @return void
 */
void OLED::init(int power, int rate, int ratio)
{
    u8g2.begin();
    currentItem[POWERLEVEL].value = power;
    currentItem[PKTRATE].value = rate;
    currentItem[TLMRADIO].value = ratio;
}

/**
 * Displays the ExpressLRS logo
 *
 * @param values power = power level char array (100 mW)
 *               rate = packet rate char array (500 hz) 
 *               ratio = telemetry rate char array (1:128) 
 * @return void
 */
void OLED::updateScreen(void)
{
    uint32_t now = millis();
    u8g2.clearBuffer();

    if(now - lastProcessTime > LOCKTIME) // LOCKTIME seconds later lock the screen
    {
        screenLocked = 1;
        u8g2.drawXBM(16, 16, 64, 64, elrs64);  
    }
    else
    {
        u8g2.setFont(u8g2_font_courR10_tr);
        for(int i=showBaseIndex;i<(4+showBaseIndex);i++)
        {
            u8g2.drawStr(0, (i-showBaseIndex)*15+12, showString[i].str); //the row line num: 12 , 27 , 42 ,57
            if(currentIndex == currentItem[i].index)
            {
                u8g2.drawStr(currentItem[i].line-10,(i-showBaseIndex)*15+12, ">");
            }
            
            u8g2.drawStr(currentItem[i].line,(i-showBaseIndex)*15+12, currentItem[i].getStr(currentItem[i].value));
        }
    }
    u8g2.sendBuffer();
}

void shortPressCB(void)
{
    if(!screenLocked)
    {
        lastProcessTime = millis();
        currentIndex ++;

        if(currentIndex > (sizeof(currentItem)/sizeof(screenShow_t) -1)) //detect last index,then comeback to first
        {
            currentIndex = 0;
            showBaseIndex =0;
        }
        if(currentIndex > 3) //each screen only shows 4 line menu
        {
            showBaseIndex ++;
        }
    }
}

void longPressCB(void)
{
    lastProcessTime = millis();
    if(screenLocked)  //unlock screen
    {
        screenLocked = 0;       
    }
    else
    {  
        currentItem[currentIndex].lpcb(currentIndex);
    }  
}

void pktRateLPCB(uint8_t i)
{
    currentItem[i].value++;

    if(currentItem[i].value>7)
    {
        currentItem[i].value = 0;
    }

    /*
    TO DO: 需要添加对应的处理部分
    */

}

void tlmLPCB(uint8_t i)
{
    currentItem[i].value++;

    if(currentItem[i].value>7)
    {
        currentItem[i].value = 0;
    }

    /*
    TO DO: 需要添加对应的处理部分
    */

}

void powerLPCB(uint8_t i)
{
    currentItem[i].value++;

    if(currentItem[i].value>7)
    {
        currentItem[i].value = 0;
    }

    /*
    TO DO: 需要添加对应的处理部分
    */

}

void rgbLPCB(uint8_t i)
{
    currentItem[i].value++;

    if(currentItem[i].value>7)
    {
        currentItem[i].value = 0;
    }

    /*
    TO DO: 需要添加对应的处理部分
    */

}

void bindLPCB(uint8_t i)
{   
    /*
    TO DO: 需要添加对应的处理部分
    */

}

void updateLPCB(uint8_t i)
{
    /*
    TO DO: 需要添加对应的处理部分
    */

}


const char * getBindString(int bind)
{
    return "bind";
}


const char * getUpdateiString(int update)
{
    return "update";
}


const char * getRgbString(int rgb)
{
    switch (rgb)
    {
    case 0: return "off";
    case 1: return "cyan";
    case 3: return "blue";
    case 4: return "green";
    case 5: return "white";
    case 6: return "violet";
    case 7: return "yellow";
    case 2: return "magenta";
    default: return "Error";
    }
}

/**
 * Returns power level string (Char array)
 *
 * @param values power = int/enum for current power level.  
 * @return const char array for power level Ex: "500 mW\0"
 */
const char *getPowerString(int power){
    switch (power)
    {
    case 0: return "10mW";
    case 1: return "25mW";
    case 3: return "100mW";
    case 4: return "250mW";
    case 5: return "500mW";
    case 6: return "1000mW";
    case 7: return "2000mW";
    case 2: return "50mW";
    default: return "Error";
    }
}

/**
 * Returns packet rate string (Char array)
 *
 * @param values rate = int/enum for current packet rate.  
 * @return const char array for packet rate Ex: "500 hz\0"
 */
const char *getRateString(int rate){
    switch (rate)
    {
    case 0: return "500hz";
    case 1: return "250hz";
    case 2: return "200hz";
    case 3: return "150hz";
    case 4: return "100hz";
    case 5: return "50hz";
    case 6: return "25hz";
    case 7: return "4hz";
    default: return "ERROR";
    }
}

/**
 * Returns telemetry ratio string (Char array)
 *
 * @param values ratio = int/enum for current power level.  
 * @return const char array for telemetry ratio Ex: "1:128\0"
 */
const char *getTLMRatioString(int ratio){
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
    default: return "error";
    }
}

/**
 * Sets the commit string by converting unsigned integers to signed char's (Char array)
 *
 * @param values commit = unsigned 8 byte int array for the commit and char array for commit string.
 * @return void
 */
void OLED::setCommitString(const uint8_t * commit, char * commitStr){

    for(int i = 0; i < 6; i++ ){
        if (commit[i] < 10) commitStr[i] = commit[i] + 48; // gets us 0 to 9
        else commitStr[i] = commit[i] + 87; // gets us a to f
    }

}

#endif
