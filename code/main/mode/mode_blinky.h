#include "ch32fun.h"

static uint8_t blinkingEnabled = 1;
static uint8_t currentBlinkPattern = 0;

#define BLINKY_NUM_MODES 10

// for random numbers
static uint32_t rand_num = 999; // Seed value

void my_srand(uint32_t seed) {
    rand_num = seed;
}

uint32_t my_rand(void) {
    // Linear Congruential Generator (LCG)
    rand_num = rand_num * 1103515245 + 12345;
    return (rand_num / 65536) % 32768;
}
void ModeBlinky_changeMode();


void check_led_unlocked(int values[]) {
    for (int i = 0; i < 6; i++) {
        if (!BadgeState_Unlocked(global_badge_state, i)) {
            values[i] = 0; //  skip leds
        }
    }
}

int counter = 0;
int max_brightness[] = {128, 128, 128, 256, 12, 128}; // purple red orange yellow green blue
int values[] = {64, 0, 0, 0, 0, 0}; // current brightness
uint32_t ticks;

void led_gatekeeping(){
    for (int i=0; i< 6; i++){
        if (!BadgeState_Unlocked(global_badge_state, i)){
            values[i] = 0;
        }
    }
}

// Timer PWM LEDs start here
void blinky_cycle() {
    LED_PWM_Pass(values[0], values[1], values[2], values[3], values[4], values[5]);
    if (SysTick->CNT - ticks > Ticks_from_Ms(200)) {
        values[counter] = 0;
        counter = (6 + counter - 1) % 6;
        
        for (int i=0; i< 6; i++){
            if (!BadgeState_Unlocked(global_badge_state, counter)){
                counter = (6 + counter - 1) % 6; //  skip leds
            }
        }
        values[counter] = max_brightness[counter];
        ticks = SysTick->CNT;
    }
}

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
            values[0] += max_brightness[0] / 64;
            values[1] += max_brightness[1] / 64;
            values[2] += max_brightness[2] / 64;
            values[3] += max_brightness[3] / 64;
            values[4] += max_brightness[4] / 12;
            values[5] += max_brightness[5] / 64;

            if (values[5] >= max_brightness[5]) {
                blinky_fade_state = 0;
            }

        } else {
            values[0] -= max_brightness[0] / 64;
            values[1] -= max_brightness[1] / 64;
            values[2] -= max_brightness[2] / 64;
            values[3] -= max_brightness[3] / 64;
            values[4] -= max_brightness[4] / 12;
            values[5] -= max_brightness[5] / 64;

            if (values[5] <= 0) {
                blinky_fade_state = 1;
            }
        }
        led_gatekeeping();
        debug_print("%d\n", max_brightness[5]);
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
            values[counter] = max_brightness[counter];
            led_gatekeeping();
        }
        
        led_gatekeeping();
        ticks = SysTick->CNT;
    }
}

void blinky_wave() {
    static uint8_t wave_position = 0;  // center of the wave (0-5)
    blinky_fade_clear();

    // wave effect with wraparound
    for (int i = 0; i < 6; i++) {
        
        int distance = i - wave_position; // distance from center

        // wraparound for circular distance
        if (distance > 3) distance = 6 - distance;   // e.g., 5-0 = 5 -> wrap to 1
        if (distance < -3) distance = 6 + distance;  // e.g., 0-5 = -5 -> wrap to 1
        if (distance < 0) distance = -distance;      // Absolute value

        // brightness based on distance from wave center
        if (distance == 0) {
            values[i] = max_brightness[i];      // Full at center
        }
        else if (distance == 1) {
            values[i] = max_brightness[i] / 2;  // Half 
        }
        else if (distance == 2) {
            values[i] = max_brightness[i] / 4;  // Quarter
        }
        else {
            values[i] = 0;  // Off
        }
    }
    led_gatekeeping();

    // Display the wave
    check_led_unlocked(values);
    LED_PWM_Pass(values[0], values[1], values[2], values[3], values[4], values[5]);

    if (SysTick->CNT - ticks > Ticks_from_Ms(250)) { // move every 250ms
        wave_position++;
        if (wave_position >= 6) {
            wave_position = 0;  // Loop back to purple
        }
        ticks = SysTick->CNT;
    }
}

typedef enum { FIREFLY_OFF, FIREFLY_IN, FIREFLY_OUT } FireflyState;
typedef struct {
    int16_t  current_brightness;
    int16_t  target_max;
    int8_t step;
    FireflyState state;
    uint32_t wait_until;
} Firefly;
static Firefly fireflies[6];

void blinky_firefly_init() {
    for (int i = 0; i < 6; i++) {
        fireflies[i].current_brightness = 0;
        fireflies[i].state = FIREFLY_OFF;
        fireflies[i].wait_until = SysTick->CNT + Ticks_from_Ms(my_rand() % 3000); // Stagger initial delays, up to 3s
    }
}

void blinky_firefly() {
    uint32_t current_tick = SysTick->CNT;
    check_led_unlocked(values);
    LED_PWM_Pass(values[0], values[1], values[2], values[3], values[4], values[5]);
    if (current_tick - ticks > Ticks_from_Ms(100)) { // 100 ms delay, handle each firefly separately
        ticks = current_tick;

        for (int i = 0; i < 6; i++) {
            switch (fireflies[i].state) {
            case FIREFLY_OFF:
                if (current_tick >= fireflies[i].wait_until) {
                    fireflies[i].state = FIREFLY_IN;
                    uint8_t brightness_percent = 50 + (my_rand() % 51); // Randomize how bright, 50-100%
                    fireflies[i].target_max = (max_brightness[i] * brightness_percent) / 100;
                    uint8_t step_percent = 1 + (my_rand() % 4); // 1-4%
                    fireflies[i].step = (max_brightness[i] * step_percent) / 100;
                    if (fireflies[i].step < 1) { fireflies[i].step = 1; } // Minimum step of 1
                    fireflies[i].current_brightness = 0;
                }
                break;

            case FIREFLY_IN:
                fireflies[i].current_brightness += fireflies[i].step;
                if (fireflies[i].current_brightness >= fireflies[i].target_max) {
                    fireflies[i].current_brightness = fireflies[i].target_max;
                    fireflies[i].state = FIREFLY_OUT;
                }
                break;

            case FIREFLY_OUT:
                fireflies[i].current_brightness -= fireflies[i].step;
                if (fireflies[i].current_brightness <= 0) {
                    fireflies[i].current_brightness = 0;
                    fireflies[i].state = FIREFLY_OFF;
                    fireflies[i].wait_until = current_tick + Ticks_from_Ms(1000 + (my_rand() % 4000)); // 1-5 seconds before glowing again
                }
                break;
            }

            // keep brightness in valid range
            if (fireflies[i].current_brightness > max_brightness[i]) { fireflies[i].current_brightness = max_brightness[i]; }
            if (fireflies[i].current_brightness < 0) { fireflies[i].current_brightness = 0; }
            values[i] = fireflies[i].current_brightness;
        }
    }
    led_gatekeeping();
}

void blinky_heartbeat() {
    static uint16_t beat_phase = 0;  // Tracks heartbeat cycle (800ms)
    blinky_fade_clear();

    int orange_center = 2; // Only originates from orange

    // Lub: 0-160ms, wave moves every 40ms
    if (beat_phase >= 0 && beat_phase < 160) {
        for (int i = 0; i < 6; i++) {
            int distance = i - orange_center;
            if (distance < 0) distance = -distance;

            // Light up LEDs based on timing
            if (beat_phase < 40 && distance == 0) {
                values[i] = max_brightness[i];  // 0-40ms: Orange only
            }
            else if (beat_phase >= 40 && beat_phase < 80 && distance == 1) {
                values[i] = max_brightness[i];  // 40-80ms: Red & Yellow only
            }
            else if (beat_phase >= 80 && beat_phase < 120 && distance == 2) {
                values[i] = max_brightness[i];  // 80-120ms: Purple & Green only
            }
            else if (beat_phase >= 120 && beat_phase < 160 && distance == 3) {
                values[i] = max_brightness[i];  // 120-160ms: Blue only
            }
        }
    }

    // Dub: 100-260ms, overlapping
    if (beat_phase >= 100 && beat_phase < 260) {
        for (int i = 0; i < 6; i++) {
            int distance = i - orange_center;
            if (distance < 0) distance = -distance;

            // 50% brightness
            if (beat_phase < 140 && distance == 0) {
                values[i] = max_brightness[i] / 2; 
            }
            else if (beat_phase >= 140 && beat_phase < 180 && distance == 1) {
                values[i] = max_brightness[i] / 2;  
            }
            else if (beat_phase >= 180 && beat_phase < 220 && distance == 2) {
                values[i] = max_brightness[i] / 2; 
            }
            else if (beat_phase >= 220 && beat_phase < 260 && distance == 3) {
                values[i] = max_brightness[i] / 2; 
            }
        }
    }
    led_gatekeeping();

    // 260-800ms all off

    // Display
    check_led_unlocked(values);
    LED_PWM_Pass(values[0], values[1], values[2], values[3], values[4], values[5]);

    // Update phase every 10ms
    if (SysTick->CNT - ticks > Ticks_from_Ms(10)) {
        beat_phase += 4;
        if (beat_phase >= 800) {
            beat_phase = 0;  // Reset cycle
        }
        ticks = SysTick->CNT;
    }
}


// GPIO blinkies start here
const char led_colors[] = { 'P', 'R', 'O', 'Y', 'G', 'B' };
void random_sparkle() {
    static uint8_t brightness = 2;
    static uint8_t current_led = 5;
    static int rain_last = 3;

    // If brightness is 0, spawn new drop
    if (brightness == 0) {
        allLedsOff();
        while (current_led == rain_last) {
            current_led = my_rand() % 6;
        }
        while (!BadgeState_Unlocked(global_badge_state, current_led)) {
            current_led = (current_led + 1) % 6;
        }
        rain_last = current_led;
        brightness = 2;
    }

    // brightness values: 2, 4, 8, 16, 32, 64, 128, 256
    uint8_t percent = (brightness > 100) ? 100 : (brightness * 100) / 256;

    // Display current brightness using GPIO PWM
    lightUpLed(led_colors[current_led], percent, 200); // Update every 200ms

    // Exponentially increase brightness
    if (brightness >= 256) {
        // Drop finished, turn off and reset
        allLedsOff();
        brightness = 0;
    }
    else {
        brightness = brightness << 1;  // Double the brightness
    }

}

void nobody_likes_crypto() { // Blink yellow 100% brightness for 50-500ms
    static uint16_t duration_ms = 50;
    static int crypto_state = 1;
    
    if (duration_ms <= 50) {
        crypto_state = 1;  // Start slowing down (getting slower)
    }
    else if (duration_ms >= 500) {
        crypto_state = 0;  // Start speeding up (getting faster)
    }

    // blinking faster or slower
    if (crypto_state) {
        lightUpLed('Y', 100, duration_ms);  
        duration_ms = duration_ms + (duration_ms/10);
    }
    else {
        lightUpLed('Y', 100, duration_ms);  
        duration_ms = (duration_ms * 10) / 11;
    }
    Delay_Ms(50);
}

void ping_pong() {
    if (!BadgeState_Unlocked(global_badge_state, 6)){
        ModeBlinky_changeMode();
        return;
    }

    static uint8_t pp_position = 0;
    static uint8_t pp_state = 1;
    lightUpLed(led_colors[pp_position], 80, 200); // Update every 500ms

    if (pp_state) { pp_position++; }
    else { pp_position--; }

    if (pp_position <= 0) { pp_state = 1; } // Start counting up 
    else if (pp_position >= 5) { pp_state = 0; }  // Start counting down
}


// Display counter in binary (bit 0 = purple, bit 1 = red, ..., bit 5 = blue), 200ms tick
void binary_counter() {
    if (!BadgeState_Unlocked(global_badge_state, 7)){
        ModeBlinky_changeMode();
        return;
    }

    static int counter = 0;
    static int counter_state = 0;

    // Turn off all LEDs first
    allLedsOff();

    // Turn on relevant bits and increment counter
    for (uint8_t i = 0; i < 6; i++) {
        if (counter & (1 << i)) {
            switch (i) {
                case 0: funDigitalWrite(LED_PURPLE, LED_ON); break;
                case 1: funDigitalWrite(LED_RED, LED_ON); break;
                case 2: funDigitalWrite(LED_ORANGE, LED_ON); break;
                case 3: funDigitalWrite(LED_YELLOW, LED_ON); break;
                case 4: funDigitalWrite(LED_GREEN, LED_ON); break;
                case 5: funDigitalWrite(LED_BLUE, LED_ON); break;
            }
        }
    }

    if (counter_state) {counter++; }
    else { counter--; }

    if (counter <= 0) { counter_state = 1; } // Start counting up 
    else if (counter > 63) { counter_state = 0; }  // Start counting down

    Delay_Ms(333);
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
        blinky_fade();
        break;
    case 2:
        blinky_pile();
        break;
    case 3:
        blinky_wave();
        break;
    case 4:
        blinky_heartbeat();
        break;
    case 5:
        blinky_firefly();
        break;
    case 6:
        gpioLedInit();
        nobody_likes_crypto();
        break;
    case 7:
        random_sparkle();
        break;
    case 8:
        ping_pong();
        break;
    case 9:
        binary_counter();
        break;
    }
}

void ModeBlinky_Setup_Pattern(){
    allLedsOff();
    if (currentBlinkPattern < 5){
        LED_PWM_Clear();
        led_pwm_gpio_init();
        blinky_fade_clear();
        ticks = SysTick->CNT;
        if (currentBlinkPattern == 2) {
            counter = 0;
        }
    }
    else if (currentBlinkPattern == 5) {    
        LED_PWM_Clear();
        led_pwm_gpio_init();
        blinky_fade_clear();
        ticks = SysTick->CNT;
        blinky_firefly_init();
    }
    else {
        LED_PWM_Clear();
        gpioLedInit();
        allLedsOff();
        
        // funDigitalWrite(LED_PURPLE, LED_ON);
        // funDigitalWrite(LED_RED, LED_ON);
        // funDigitalWrite(LED_ORANGE, LED_ON);
        // funDigitalWrite(LED_YELLOW, LED_ON);
        // funDigitalWrite(LED_GREEN, LED_ON);
        // funDigitalWrite(LED_BLUE, LED_ON);

        uint32_t ticks = SysTick->CNT;
        while (SysTick->CNT - ticks < Ticks_from_Ms(1000)) {}
        
        allLedsOff();
        
        my_srand(SysTick->CNT); // seed for random rand() numbers
    };

}

void ModeBlinky_Setup(){
    currentBlinkPattern = 0;
    ModeBlinky_Setup_Pattern();
}

void ModeBlinky_changeMode(){
    currentBlinkPattern = (currentBlinkPattern + 1) % BLINKY_NUM_MODES;

    // if (currentBlinkPattern == 8 && !BadgeState_Unlocked(global_badge_state, 6)){
    //     currentBlinkPattern = 9;
    // }
    // if (currentBlinkPattern == 9 && !BadgeState_Unlocked(global_badge_state, 7)){
    //     currentBlinkPattern = 0;
    // }
    ModeBlinky_Setup_Pattern();
}
