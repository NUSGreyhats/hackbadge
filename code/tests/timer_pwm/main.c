/*
 * Example for using Advanced Control Timer (TIM1) for PWM generation
 * 03-28-2023 E. Brombaugh
 */

#include "ch32fun.h"
#include <stdio.h>
#include "pinout.h"
#include "timers_led.h"
int main()
{
    uint32_t count = 0;
    
    SystemInit();
    Delay_Ms( 100 );

    gpio_init();
    t1pwm_init();
    t2pwm_init();

    count = 8;

    uint32_t ticks = SysTick->CNT;
    while(1)
        {
        // displayLEDPWM(255, 255, 255, 255, 255, 255);
        displayLEDPWM(64, count, 64, count, 16, count);
        // displayLEDPWM(0,255,0,0,0,0);
        if (SysTick->CNT - ticks > Ticks_from_Ms(2000)) {
            break;
        }
    }

    ticks = SysTick->CNT;
    int counter = 0;
    int og[] = {64, 64, 64, 128, 16, 64};
    // int og[] = {255, 255, 255, 255, 255, 255};
    int values[] = {0, 0, 0, 0, 0, 0};
    while(1)
        {
        // displayLEDPWM(255, count, 255, count, 128, count);
        displayLEDPWM(values[0], values[1], values[2], values[3], values[4], values[5]);

        if (SysTick->CNT - ticks > Ticks_from_Ms(200)) {
            values[counter] = 0;
            counter = (counter + 1) % 6;
            values[counter] = og[counter];
            ticks = SysTick->CNT;
        }
    }
}