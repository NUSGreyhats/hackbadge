#include "ch32fun.h"

static uint8_t blinkingEnabled = 1;
static uint8_t currentBlinkPattern = 0;
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