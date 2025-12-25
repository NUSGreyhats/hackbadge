#include "ch32fun.h"
#include <string.h>

#include "pinout.h"  

// LEDs are active LOW (GPIO LOW = LED ON)
#define LED_ON   FUN_LOW
#define LED_OFF  FUN_HIGH

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
