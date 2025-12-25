#include "ch32fun.h"

static uint8_t blinkingEnabled = 1;
static uint8_t currentBlinkPattern = 0;


#define BLINKY_NUM_MODES 5


int counter = 0;
int og[] = {64, 64, 64, 128, 16, 64};
// int og[] = {255, 255, 255, 255, 255, 255};
int values[] = {64, 0, 0, 0, 0, 0};
uint32_t ticks;

// Blink patterns
void runBlinkPattern(uint8_t pattern)
{   
    switch (pattern)
    {
    case 0:
        LED_PWM_Pass(values[0], values[1], values[2], values[3], values[4], values[5]);
        if (SysTick->CNT - ticks > Ticks_from_Ms(200)) {
            values[counter] = 0;
            counter = (6 + counter - 1) % 6;
            values[counter] = og[counter];
            ticks = SysTick->CNT;
        }
        break;
    case 1:
        // Blue blink
        lightUpLed("blue", 10, 1);
        Delay_Ms(300); // Pause between blinks
        break;
    case 2:
        // Yellow blink
        lightUpLed("yellow", 10, 1);
        Delay_Ms(300); // Pause between blinks
        break;
    case 3:
        // Alternating purple + blue
        lightUpLed("purple", 10, 1);
        lightUpLed("blue", 100, 2);
        Delay_Ms(300); // Pause between blinks
        break;
    case 4:
        // Purple blink
        lightUpLed("purple", 10, 1);
        Delay_Ms(300); // Pause between blinks
        break;
    }
}

void ModeBlinky_Setup_Pattern(){
    if (currentBlinkPattern == 0){
        led_pwm_gpio_init();
        LED_PWM_Init();
        ticks = SysTick->CNT;
    }else {
        gpioLedInit();
    };

}

void ModeBlinky_Setup(){
    ModeBlinky_Setup_Pattern();
}

void ModeBlinky_changeMode(){
    currentBlinkPattern = (currentBlinkPattern + 1) % BLINKY_NUM_MODES;
    ModeBlinky_Setup_Pattern();
}