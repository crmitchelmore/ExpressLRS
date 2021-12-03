#pragma once

#include <functional>

#define ADC_VCC     (4095)    //the ADC output value VCC ,the 5D button was not pressed
#define ADC_RIGHT   (3511)    //The ADC sampling values when right of 5D button was pressed 
#define ADC_UP      (2839)    //The ADC sampling values when up of 5D button was pressed
#define ADC_DOWN    (2191)    //The ADC sampling values when down of 5D button was pressed
#define ADC_LEFT    (1616)    //The ADC sampling values when left of 5D button was pressed
#define ADC_MIDDLE  (0)       //The ADC sampling values when middle of 5D button was pressed

#define ADC_BOUNDARY_RIGHT      ((ADC_VCC + ADC_RIGHT)/2)
#define ADC_BOUNDARY_UP		    ((ADC_RIGHT + ADC_UP)/2)
#define ADC_BOUNDARY_DOWN		((ADC_UP + ADC_DOWN)/2)
#define ADC_BOUNDARY_LEFT		((ADC_DOWN + ADC_LEFT)/2)
#define ADC_BOUNDARY_MIDDLE		((ADC_LEFT + ADC_MIDDLE)/2)

const uint16_t buttonAdcBoundaryTable[] =
{
    ADC_BOUNDARY_MIDDLE,ADC_BOUNDARY_LEFT,ADC_BOUNDARY_DOWN,ADC_BOUNDARY_UP,ADC_BOUNDARY_RIGHT
};
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
    uint16_t adcValue;
    
    // button debounce
    uint8_t preDebounceButton;
    uint8_t afterDebounceButton;
    uint16_t DebounceTriggerTime;
    
    //button scan
    uint8_t lastButton;
    uint16_t buttonPressedTime;
} button_5d_t;

template<uint8_t PIN, bool IDLELOW>
class Button_5d
{
private:
    /* data */
    static constexpr uint32_t MS_DEBOUNCE = 25;       // how long the switch must change state to be considered
    static constexpr uint32_t MS_LONG = 500;
    static constexpr uint32_t MS_HOLD = 500;
    button_5d_t button;
public:
    // Callbacks
    std::function<void ()>OnButtonUpShortPress;
    std::function<void ()>OnButtonUpLongPress;

    std::function<void ()>OnButtonDownShortPress;
    std::function<void ()>OnButonDownLongPress;

    std::function<void ()>OnButtonLeftShortPress;
    std::function<void ()>OnButtonLeftLongPress;

    std::function<void ()>OnButtonRightShortPress;
    std::function<void ()>OnButtonRightLongPress;

    std::function<void ()>OnButtonMiddleShortPress;
    std::function<void ()>OnButtonMiddleLongPress;

    Button_5d() 
    {
        pinMode(PIN, IDLELOW ? INPUT : INPUT_PULLUP);
    }

    int update()
    {
        sampleButton(&button);
        return MS_DEBOUNCE;
    }
    void sampleButton(button_5d_t *button)
    {
        uint8_t currentButton;
        currentButton = BUTTON_NONE;
        button->adcValue = analogRead(PIN);
        currentButton = buttonFilter(button,getButtonOrientation(button->adcValue));
        if (currentButton == button->lastButton)     //The state of the key has not changed（keep pressd or released）
        {
            if (currentButton == BUTTON_NONE)         //If current button is in release,just return and do nothing
            {
                return;
            }
            button->buttonPressedTime+=MS_DEBOUNCE;
            if (button->buttonPressedTime == MS_LONG)          //The button is pressed for a long time
            {
                switch(currentButton)
                {
                    case BUTTON_MODDLE:
                        if(OnButtonMiddleLongPress)
                            OnButtonMiddleLongPress();
                        break;
                    case BUTTON_LEFT:
                        if(OnButtonLeftLongPress)
                            OnButtonLeftLongPress();
                        break;
                    case BUTTON_DOWN:
                        if(OnButonDownLongPress)
                            OnButonDownLongPress();
                        break;
                    case BUTTON_UP:
                        if(OnButtonUpLongPress)
                            OnButtonUpLongPress();
                        break;
                    case BUTTON_RIGHT:
                        if(OnButtonRightLongPress)
                            OnButtonRightLongPress();
                        break;
                    case BUTTON_NONE:
                        break;
                    default:
                        break;
                }
            }
            else if (button->buttonPressedTime == (MS_LONG + MS_HOLD))   //If hold pressed,run short press callback in cycles
            {
                button->buttonPressedTime = MS_LONG;
                switch(currentButton)
                {
                    case BUTTON_MODDLE:
                        if(OnButtonMiddleShortPress)
                            OnButtonMiddleShortPress();
                        break;
                    case BUTTON_LEFT:
                        if(OnButtonLeftShortPress)
                            OnButtonLeftShortPress();
                        break;
                    case BUTTON_DOWN:
                        if(OnButtonDownShortPress)
                            OnButtonDownShortPress();
                        break;
                    case BUTTON_UP:
                        if(OnButtonUpShortPress)
                            OnButtonUpShortPress();
                        break;
                    case BUTTON_RIGHT:
                        if(OnButtonRightShortPress)
                            OnButtonRightShortPress();
                        break;
                    case BUTTON_NONE:
                        break;
                    default:
                        break;
                }
            }
            else
            {
                return;
            }
        }
        else  //The state of the button is changed
        {
            if ((button->buttonPressedTime < MS_LONG) && (currentButton != BUTTON_NONE))//The button goes from being released to being pressed
            {
                    button->buttonPressedTime = 0; 
            }
            if((button->buttonPressedTime < MS_LONG) && (currentButton == BUTTON_NONE))//The button is released after a short press
            {
                switch(button->lastButton)
                {
                    case BUTTON_MODDLE:
                        if(OnButtonMiddleShortPress)
                            OnButtonMiddleShortPress();
                        break;
                    case BUTTON_LEFT:
                        if(OnButtonLeftShortPress)
                            OnButtonLeftShortPress();
                        break;
                    case BUTTON_DOWN:
                        if(OnButtonDownShortPress)
                            OnButtonDownShortPress();
                        break;
                    case BUTTON_UP:
                        if(OnButtonUpShortPress)
                            OnButtonUpShortPress();
                        break;
                    case BUTTON_RIGHT:
                        if(OnButtonRightShortPress)
                            OnButtonRightShortPress();
                        break;
                    case BUTTON_NONE:
                        break;
                    default:
                        break;
                }
            }
            else if((button->buttonPressedTime >= MS_LONG) && (currentButton == BUTTON_NONE) ) //The button is released after a long press
            {
                    button->buttonPressedTime = 0;
                    switch(button->lastButton)
                {
                    case BUTTON_MODDLE:
                        break;
                    case BUTTON_LEFT:
                        break;
                    case BUTTON_DOWN:
                        break;
                    case BUTTON_UP:
                        break;
                    case BUTTON_RIGHT:
                        break;
                    case BUTTON_NONE:
                        break;
                    default:
                        break;
                }
            }
            else
            {
                button->buttonPressedTime = 0;
                button->lastButton = currentButton;
                return;
            }
            button->lastButton = currentButton;
        }
    }
    
    uint8_t getButtonOrientation(uint16_t buttonAdcValue)
    {
        uint8_t btnNumber = 0;
    
        if (buttonAdcValue > ADC_BOUNDARY_RIGHT)   return BUTTON_NONE;
    
        for (btnNumber = 0; btnNumber < sizeof (buttonAdcBoundaryTable) / sizeof (buttonAdcBoundaryTable[0]); btnNumber++)
        {	
            if (buttonAdcValue < buttonAdcBoundaryTable[btnNumber])
            break;
        }
        return btnNumber;
    }

    //button debounce
    uint8_t buttonFilter(button_5d_t *button,uint8_t buttonNum)
    {
        if (button->preDebounceButton != buttonNum)
        {
            button->DebounceTriggerTime = 0;
            button->preDebounceButton = buttonNum;
        }
        else
        {
            button->DebounceTriggerTime += MS_DEBOUNCE;
            if (button->DebounceTriggerTime > MS_DEBOUNCE)
            {
                button->afterDebounceButton = buttonNum;
            }
        }
        return button->afterDebounceButton;
    }
};

