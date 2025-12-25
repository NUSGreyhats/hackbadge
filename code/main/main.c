#include "ch32fun.h"
#include <stdio.h>
#include <string.h>
#include "pinout.h"  
#include "timers_led.h"

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

// Button state tracking
typedef struct {
    uint8_t is_pressed;
    uint8_t was_pressed;
    uint32_t press_time_ms;
    uint8_t long_press_triggered;
} ButtonState;

static ButtonState btn1 = { 0 };
static ButtonState btn2 = { 0 };

// Helper: active-low buttons; returns 1 if pressed
static uint8_t readButton(int pin)
{
    return funDigitalRead(pin) == 0;
}

// Turn off all LEDs (write HIGH since active-low)
void allLedsOff()
{
    funDigitalWrite(LED_PURPLE, LED_OFF);
    funDigitalWrite(LED_RED, LED_OFF);
    funDigitalWrite(LED_GREEN, LED_OFF);
    funDigitalWrite(LED_BLUE, LED_OFF);
    funDigitalWrite(LED_YELLOW, LED_OFF);
    funDigitalWrite(LED_ORANGE, LED_OFF);
}

// Software PWM for one pin
// brightness: 0..100
// duration_s: seconds (integer)
void lightUpLed(const char* color, uint8_t brightness, uint16_t duration_s)
{
    int pin;

    if (!strcmp(color, "purple"))      pin = LED_PURPLE;
    else if (!strcmp(color, "blue"))   pin = LED_BLUE;
    else if (!strcmp(color, "yellow")) pin = LED_YELLOW;
    else if (!strcmp(color, "orange")) pin = LED_ORANGE;
    else return;

    // Turn off all LEDs first
    allLedsOff();

    if (brightness == 0) {
        return;
    }
    if (brightness >= 100) {
        funDigitalWrite(pin, LED_ON);  // Active LOW - write LOW to turn on
        for (uint32_t t = 0; t < (uint32_t)duration_s * 100; t++) {
            Delay_Ms(10);
        }
        funDigitalWrite(pin, LED_OFF);
        return;
    }

    // Software PWM parameters
    const uint8_t period_ms = 20;    // 20ms period (50Hz)
    const uint8_t slice_ms = 2;      // 2ms resolution
    const uint8_t slices = period_ms / slice_ms; // 10 slices
    uint8_t on_slices = (brightness * slices) / 100;

    // Run for duration_s seconds
    uint32_t iterations = (duration_s * 1000) / period_ms;
    for (uint32_t it = 0; it < iterations; it++)
    {
        for (uint8_t s = 0; s < slices; s++)
        {
            if (s < on_slices) {
                funDigitalWrite(pin, LED_ON);  // Active LOW
            }
            else {
                funDigitalWrite(pin, LED_OFF);
            }
            Delay_Ms(slice_ms);
        }
    }
    funDigitalWrite(pin, LED_OFF);
}

// Update button state and return event type
// Returns: 0=nothing, 1=short press, 2=long press
uint8_t updateButton(int pin, ButtonState* state, uint16_t loop_delay_ms)
{
    state->is_pressed = readButton(pin);

    if (state->is_pressed) {
        if (!state->was_pressed) {
            // Button just pressed
            state->was_pressed = 1;
            state->press_time_ms = 0;
            state->long_press_triggered = 0;
        }
        else {
            // Button continues to be held
            state->press_time_ms += loop_delay_ms;

            // Check for long press (5 seconds)
            if (state->press_time_ms >= 5000 && !state->long_press_triggered) {
                state->long_press_triggered = 1;
                return 2; // Long press detected
            }
        }
    }
    else {
        // Button not pressed
        if (state->was_pressed) {
            // Button just released
            state->was_pressed = 0;

            // Check if it was a short press (100ms to 5000ms)
            if (state->press_time_ms >= 100 && state->press_time_ms < 5000) {
                state->press_time_ms = 0;
                return 1; // Short press detected
            }

            state->press_time_ms = 0;
        }
    }

    return 0; // No event
}

// Flash all LEDs for 1 second (mode change indicator)
void flashAllLeds()
{
    //funDigitalWrite(LED_PURPLE, LED_ON);
    //funDigitalWrite(LED_BLUE, LED_ON);
    //funDigitalWrite(LED_YELLOW, LED_ON);
    lightUpLed("orange", 10, 1);
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
                funDigitalWrite(LED_RED, LED_ON);
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