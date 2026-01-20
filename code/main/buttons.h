#include "ch32fun.h"

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
