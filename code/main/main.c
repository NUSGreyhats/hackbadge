#include "ch32fun.h"
#include <stdio.h>
#include <string.h>

#include "pinout.h"  
#include "led_timers.h"
#include "led_gpio.h"
#include "buttons.h"

// Button pin definitions
#define BTN1   BTN_LEFT
#define BTN2   BTN_RIGHT

// LEDs are active LOW (GPIO LOW = LED ON)
#define LED_ON   FUN_LOW
#define LED_OFF  FUN_HIGH

// Modes
typedef enum {
    MODE_BLINK = 0,
    MODE_CTF = 1
} SystemMode;

static SystemMode currentMode = MODE_BLINK;
static uint8_t blinkingEnabled = 1;
static uint8_t currentBlinkPattern = 0;


// Flash all LEDs for 1 second (mode change indicator)
void flashAllLeds()
{
    //funDigitalWrite(LED_PURPLE, LED_ON);
    //funDigitalWrite(LED_BLUE, LED_ON);
    //funDigitalWrite(LED_YELLOW, LED_ON);
    lightUpLed("blue", 10, 1);
    Delay_Ms(1000);
    allLedsOff();
}

// Blink patterns
void runBlinkPattern(uint8_t pattern)
{
    switch (pattern)
    {
    case 0:
        // Purple blink
        lightUpLed("purple", 10, 1);
        break;
    case 1:
        // Blue blink
        lightUpLed("blue", 10, 1);
        break;
    case 2:
        // Yellow blink
        lightUpLed("yellow", 10, 1);
        break;
    case 3:
        // Alternating purple + blue
        lightUpLed("purple", 10, 1);
        lightUpLed("blue", 100, 2);
        break;
    }
    Delay_Ms(300); // Pause between blinks
}

int main(void)
{
    SystemInit();
    funGpioInitAll();

    // Configure LEDs as outputs
    funPinMode(LED_PURPLE, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);
    funPinMode(LED_RED,    GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);
    funPinMode(LED_ORANGE, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);
    funPinMode(LED_YELLOW, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);
    funPinMode(LED_GREEN,  GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);
    funPinMode(LED_BLUE,   GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);

    // Configure buttons with pull-up
    funPinMode(BTN1, GPIO_CNF_IN_PUPD);
    funDigitalWrite(BTN1, FUN_HIGH);
    funPinMode(BTN2, GPIO_CNF_IN_PUPD);
    funDigitalWrite(BTN2, FUN_HIGH);

    // Turn off all LEDs initially (write HIGH for active-low)
    allLedsOff();
    const uint16_t LOOP_DELAY_MS = 50; // Poll buttons every 50ms

    while (1)
    {
        // Update button states
        uint8_t btn1_event = updateButton(BTN1, &btn1, LOOP_DELAY_MS);
        uint8_t btn2_event = updateButton(BTN2, &btn2, LOOP_DELAY_MS);

        // Handle BTN1 events - Mode switch
        if (btn1_event == 2) {
            currentMode = (currentMode == MODE_BLINK) ? MODE_CTF : MODE_BLINK;
            flashAllLeds();
        }

        // Handle BTN2 events - Only in BLINK mode
        if (currentMode == MODE_BLINK) {
            // Long press: toggle blinking
            if (btn2_event == 2) {
                blinkingEnabled = !blinkingEnabled;
                flashAllLeds();
            }

            // Short press: cycle pattern (only when blinking enabled)
            if (btn2_event == 1 && blinkingEnabled) {
                currentBlinkPattern = (currentBlinkPattern + 1) % 4;
            }
        }

        // Execute mode behavior (only if no buttons currently pressed)
        if (!btn1.is_pressed && !btn2.is_pressed) {
            if (currentMode == MODE_BLINK && blinkingEnabled) {
                funDigitalWrite(LED_PURPLE, LED_ON);
                runBlinkPattern(currentBlinkPattern);
            }
            else if (currentMode == MODE_CTF) {
                // funDigitalWrite(LED_RED, LED_ON);
                // CTF mode - dim purple breathing
                lightUpLed("purple", 20, 1);
                Delay_Ms(500);
            }
            else {
                // Blink mode but disabled - keep LEDs off
                allLedsOff();
                Delay_Ms(LOOP_DELAY_MS);
            }
        }
        else {
            // Buttons being pressed, just wait
            Delay_Ms(LOOP_DELAY_MS);
        }
        
    }
}