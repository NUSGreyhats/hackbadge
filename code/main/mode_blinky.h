#include "ch32fun.h"

static uint8_t blinkingEnabled = 1;
static uint8_t currentBlinkPattern = 0;


#define BLINKY_NUM_MODES 5

int counter = 0;
int og[] = {64, 64, 64, 128, 16, 64};
int values[] = {64, 0, 0, 0, 0, 0};
uint32_t ticks;
void blinky_cycle() {
    LED_PWM_Pass(values[0], values[1], values[2], values[3], values[4], values[5]);
    if (SysTick->CNT - ticks > Ticks_from_Ms(200)) {
        values[counter] = 0;
        counter = (6 + counter - 1) % 6;
        values[counter] = og[counter];
        ticks = SysTick->CNT;
    }
}

int blinky_fade_max[] = {64, 64, 64, 128, 16, 64};
char blinky_fade_state = 1;
void blinky_fade_clear() {
    values[0] = 0;
    values[1] = 0;
    values[2] = 0;
    values[3] = 0;
    values[4] = 0;
    values[5] = 0;
}
void blinky_fade() {
    LED_PWM_Pass(values[0], values[1], values[2], values[3], values[4], values[5]);
    if (SysTick->CNT - ticks > Ticks_from_Ms(50)) {
        if (blinky_fade_state){
            values[0] += 1;
            values[1] += 1;
            values[2] += 1;
            values[3] += 2;
            values[4] += 1;
            values[5] += 1; 

            if (values[0] >= 64) {
                blinky_fade_state = 0;
            }

        } else {
            values[0] -= 1;
            values[1] -= 1;
            values[2] -= 1;
            values[3] -= 2;
            values[4] -= 1;
            values[5] -= 1;

            if (values[0] <= 0) {
                blinky_fade_state = 1;
            }
        }
        
        ticks = SysTick->CNT;
    }
}


void blinky_pile() {
    LED_PWM_Pass(values[0], values[1], values[2], values[3], values[4], values[5]);
    if (SysTick->CNT - ticks > Ticks_from_Ms(200)) {
        counter--;
        if (counter < 0) {
            blinky_fade_clear();
            counter = 6;
        } else {
            values[counter] = og[counter];
        }
        
        ticks = SysTick->CNT;
    }
}

// Blink patterns
void runBlinkPattern(uint8_t pattern)
{   
    switch (pattern)
    {
    case 0:
        blinky_cycle();
        break;
    case 1:
        blinky_pile();
        break;
    case 2:
        blinky_fade();
        break;
    case 3:
        gpioLedInit();
        // Yellow blink
        lightUpLed("yellow", 10, 1);
        Delay_Ms(300); // Pause between blinks
        break;
    case 4:
        // Alternating purple + blue
        lightUpLed("purple", 10, 1);
        lightUpLed("blue", 100, 2);
        Delay_Ms(300); // Pause between blinks
        break;
    case 5:
        // Purple blink
        lightUpLed("purple", 10, 1);
        Delay_Ms(300); // Pause between blinks
        break;
    }
}

void ModeBlinky_Setup_Pattern(){
    allLedsOff();
    if (currentBlinkPattern == 0){
        led_pwm_gpio_init();
        ticks = SysTick->CNT;
    }else if (currentBlinkPattern == 1){
        led_pwm_gpio_init();
        ticks = SysTick->CNT;
    }else if (currentBlinkPattern == 2){
        led_pwm_gpio_init();
        ticks = SysTick->CNT;
    }else {
        gpioLedInit();
        allLedsOff();
    };

}

void ModeBlinky_Setup(){
    ModeBlinky_Setup_Pattern();
}

void ModeBlinky_changeMode(){
    currentBlinkPattern = (currentBlinkPattern + 1) % BLINKY_NUM_MODES;
    ModeBlinky_Setup_Pattern();
}