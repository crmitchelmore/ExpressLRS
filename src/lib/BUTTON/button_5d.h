#pragma once

#include "targets.h"
typedef enum{
    BUTTON_NONE_PRESS = 0,
    BUTTON_SHORT_PRESS,
    BUTTON_DOUBLE_PRESS,
    BUTTON_THREE_PRESS,
    BUTTON_LONG_PRESS,
    BUTTON_UP_AFTER_LONG_PRESS,
    BUTTON_HOLD_PRESS,
}button_state_e;

typedef enum{    
    BUTTON_MODDLE,
    BUTTON_LEFT,
    BUTTON_DOWN,
    BUTTON_UP,
    BUTTON_RIGHT,
    BUTTON_NONE = 0XFF,
}button_orientation_e;

typedef struct
{
    uint16_t adc_value;

    //filter
    uint8_t used_button;
    uint8_t old_button;
    uint16_t button_counter;

    //scan
    uint8_t last_button;
    uint16_t button_press_counter;
    uint8_t button_press_flag;

} button_5d_t;


class button_5d
{
private:
    static int buttonPin;
   
    static void sampleButton(button_5d_t *button);
    static uint8_t getButtonOrientation(uint16_t button_adc_value);
    static uint8_t buttonFilter(button_5d_t *button,uint8_t button_num);

public:

    static void init(int Pin);
    static void handle();
   
    static void inline nullCallback(void);

    static void (*buttonUpShortPress)();
    static void (*buttonUpLongPress)();

    static void (*buttonDownShortPress)();
    static void (*buttonDownLongPress)();

    static void (*buttonLeftShortPress)();
    static void (*buttonLeftLongPress)();

    static void (*buttonRightShortPress)();
    static void (*buttonRightLongPress)();

    static void (*buttonMiddleShortPress)();
    static void (*buttonMiddleLongPress)();

};
