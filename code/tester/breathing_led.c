/*
 * RAW ADC Monitor - Zero Processing
 * Shows direct ADC readings from PA2 with no baseline tracking,
 * no deviation calculation, just pure voltage measurements
 *
 * This reveals:
 * - Actual DC bias voltage on PA2
 * - Whether internal pull-up/pull-down affects readings
 * - Raw AC + DC signal from microphone
 */

#include "ch32fun.h"
#include <stdio.h>

#define ADC_NUMCHLS 1
volatile uint16_t adc_buffer[ADC_NUMCHLS];

// LED pins for raw value display
#define LED_PURPLE   PD3
#define LED_BLUE     PC4
#define LED_GREEN    PC5
#define LED_YELLOW   PC6
#define LED_ORANGE   PC7
#define LED_RED      PD2

#define BTN2         PD0
#define BTN1         PD1
  
int main(void)
{
    SystemInit();

    funGpioInitAll();
    funPinMode(LED_PURPLE, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);
    funPinMode(LED_BLUE,   GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);
    funPinMode(LED_YELLOW, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);
    funPinMode(LED_ORANGE, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);
    funPinMode(LED_GREEN,  GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);
    funPinMode(LED_RED,    GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);

    int leds[] = {LED_BLUE, LED_GREEN, LED_YELLOW, LED_ORANGE, LED_RED, LED_PURPLE};
    int counter = 0;
    while (1)
    {
        funDigitalWrite(LED_ORANGE, FUN_HIGH);
        funDigitalWrite(LED_YELLOW, FUN_HIGH);
        funDigitalWrite(LED_BLUE, FUN_HIGH);
        funDigitalWrite(LED_PURPLE, FUN_HIGH);
        funDigitalWrite(LED_GREEN, FUN_HIGH);
        funDigitalWrite(LED_RED, FUN_HIGH);


        // Delay_Ms(1000);

        // funDigitalWrite(LED_ORANGE, FUN_LOW);
        // funDigitalWrite(LED_YELLOW, FUN_LOW);
        // funDigitalWrite(LED_BLUE,   FUN_LOW);
        // funDigitalWrite(LED_PURPLE, FUN_LOW);
        // funDigitalWrite(LED_GREEN, FUN_LOW);
        // funDigitalWrite(LED_RED, FUN_LOW);
        funDigitalWrite(leds[counter], FUN_LOW);
        
        Delay_Ms(200);

        counter = (counter + 1) % 6;
    }
}