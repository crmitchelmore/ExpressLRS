#include "button_5d.h"

#define KEY_LONG_CNT  2000    //how long the switch must hold state to be considered a long press
#define KEY_HOLD_CNT  500     //how long the switch must hold state (after KEY_LONG_CNT + KEY_HOLD_CNT) to be considered a hold press
#define KEY_DEBOUNCE_CNT 50   //how long the switch must change state to be considered

#define ADC_VCC (4095)        //the ADC output value VCC ,the 5D button was not pressed

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

int button_5d::buttonPin = -1;
static button_5d_t button;
void inline button_5d::nullCallback(void) {}

void (*button_5d::buttonUpShortPress)() = &nullCallback;
void (*button_5d::buttonUpLongPress)() = &nullCallback;

void (*button_5d::buttonDownShortPress)() = &nullCallback;
void (*button_5d::buttonDownLongPress)() = &nullCallback;

void (*button_5d::buttonLeftShortPress)() = &nullCallback;
void (*button_5d::buttonLeftLongPress)() = &nullCallback;

void (*button_5d::buttonRightShortPress)() = &nullCallback;
void (*button_5d::buttonRightLongPress)() = &nullCallback;

void (*button_5d::buttonMiddleShortPress)() = &nullCallback;
void (*button_5d::buttonMiddleLongPress)() = &nullCallback;

const uint16_t button_adc_boundary_table[] =
{
    ADC_BOUNDARY_MIDDLE,ADC_BOUNDARY_LEFT,ADC_BOUNDARY_DOWN,ADC_BOUNDARY_UP,ADC_BOUNDARY_RIGHT
};

void button_5d::init(int Pin)
{
   
    pinMode(Pin, INPUT);
    buttonPin = Pin;


}

void button_5d::handle()
{
    sampleButton(&button);
}


void button_5d::sampleButton(button_5d_t *button)
{
    uint8_t current_button;
 
    current_button = BUTTON_NONE;
    
    button->adc_value = analogRead(buttonPin);
    //button debounce
    current_button = buttonFilter(button,getButtonOrientation(button->adc_value));
    if (current_button == button->last_button)     //The state of the key has not changed（keep pressd or released）
    {
        if (current_button == BUTTON_NONE)         //The button is released
        {
            return;
        }
        button->button_press_counter++;
        if (button->button_press_counter == KEY_LONG_CNT)          //The button is pressed for a long time
        {

            switch(current_button)
            {
                case BUTTON_MODDLE:
                    buttonMiddleLongPress();
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
        else if (button->button_press_counter == (KEY_LONG_CNT + KEY_HOLD_CNT))        //The button is always pressed
        {
            button->button_press_counter = KEY_LONG_CNT;
        }
        else
        {
            return;
        }
    }
    else  //The state of the button is changed
    {
        
        if ((button->button_press_counter < KEY_LONG_CNT) && (current_button != BUTTON_NONE))//The button goes from being released to being pressed
        {
                button->button_press_counter = 0; 
        }

        if((button->button_press_counter < KEY_LONG_CNT) && (current_button == BUTTON_NONE))//The button is released after a short press
        {
            switch(button->last_button)
            {
                case BUTTON_MODDLE:
                    buttonMiddleShortPress();
                    break;
                case BUTTON_LEFT:
                    buttonLeftShortPress();
                    break;
                case BUTTON_DOWN:
                    buttonDownShortPress();
                    break;
                case BUTTON_UP:
                    buttonUpShortPress();
                    break;
                case BUTTON_RIGHT:
                    buttonRightShortPress();
                    break;
                case BUTTON_NONE:
                    break;
                default:
                    break;
            }
        }
        else if((button->button_press_counter >= KEY_LONG_CNT) && (current_button == BUTTON_NONE) ) //The button is released after a long press
        {
                button->button_press_counter = 0;
                switch(button->last_button)
            {
                case BUTTON_MODDLE:
                    break;
                case BUTTON_LEFT:
                    buttonLeftLongPress();
                    break;
                case BUTTON_DOWN:
                    buttonDownLongPress();
                    break;
                case BUTTON_UP:
                    buttonUpLongPress();
                    break;
                case BUTTON_RIGHT:
                    buttonRightLongPress();
                    break;
                case BUTTON_NONE:
                    break;
                default:
                    break;
            }
        }
        else
        {
            button->button_press_counter = 0;
            button->last_button = current_button;
            return;
        }
        button->last_button = current_button;
    }

}


uint8_t button_5d::getButtonOrientation(uint16_t button_adc_value)
{
    uint8_t key_number = 0;
 
    if (button_adc_value > ADC_BOUNDARY_RIGHT)   return BUTTON_NONE;
 
    for (key_number = 0; key_number < sizeof (button_adc_boundary_table) / sizeof (button_adc_boundary_table[0]); key_number++)
	{	
		if (button_adc_value < button_adc_boundary_table[key_number])
		break;
	}

	return key_number;
}



//button debounce
uint8_t button_5d::buttonFilter(button_5d_t *button,uint8_t button_num)
{
 
    if (button->old_button != button_num)
    {
        button->button_counter = 0;
        button->old_button = button_num;
    }
    else
    {
        button->button_counter++;
        if (button->button_counter == KEY_DEBOUNCE_CNT)
        {
            button->used_button = button_num;
        }
    }
    
    return button->used_button;
}


