#include "ch32fun.h"
#include <stdio.h>
#include <string.h>

#include "pinout.h"  
#include "led_timers.h"
#include "led_gpio.h"
#include "buttons.h"
#include "mode_blinky.h"
#include "buttons_interrupts.h"

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

#define MODE_CTF_KEYLEN 8
char mode_ctf_btn_buffer[MODE_CTF_KEYLEN];
void mode_ctf_btn_buffer_shift(char new_val){
    for(int i=0; i<MODE_CTF_KEYLEN-1; i++){
        mode_ctf_btn_buffer[i+1] = mode_ctf_btn_buffer[i];
    }
    mode_ctf_btn_buffer[0] = new_val;
} 

// int mode_ctf_btn_buffer_map(){
//     int val = 0;
//     for (int i=0; i<MODE_CTF_KEYLEN; i++) {
//         if (mode_ctf_btn_buffer == -1) { return -1; } // invalid
//         val += (mode_ctf_btn_buffer[i]) << i;
//     }
//     return val;
// } 

int mode_ctf_check_and_unlock(){
    // int val = mode_ctf_btn_buffer_map();
    if (mode_ctf_btn_buffer[0] == 0 && mode_ctf_btn_buffer[1] == 1){
        return 1;
    } 
    return 0;
} 

int mode_ctf_main(){
    Interrupt_Btn_Disable();
    gpioLedInit();

    // Buffer
    memset(mode_ctf_btn_buffer, -1, MODE_CTF_KEYLEN);

    funDigitalWrite(LED_YELLOW, LED_ON);
    while (1)
    {
        // // Update button states
        uint8_t btnl_event = readButton(BTN_LEFT);
        uint8_t btnr_event = readButton(BTN_RIGHT);

        // Handle BTN1 events - Mode switch
        if (btnl_event == 1) {
            funDigitalWrite(LED_RED, LED_ON);
            Delay_Ms(100);
            mode_ctf_btn_buffer_shift(0);
            if (mode_ctf_check_and_unlock()){ break; }
            funDigitalWrite(LED_RED, LED_OFF);
        }

        if (btnr_event == 1) {
            funDigitalWrite(LED_ORANGE, LED_ON);
            Delay_Ms(100);
            mode_ctf_btn_buffer_shift(1);
            if (mode_ctf_check_and_unlock()){ break; }
            funDigitalWrite(LED_ORANGE, LED_OFF);
        }
    }

    Interrupt_Btn_Enable();
    Delay_Ms(1000);
    currentMode = MODE_BLINK;
}

int main(void)
{
    SystemInit();
    funGpioInitAll();
    LED_PWM_Init();
    gpioLedInit();

    // Configure buttons with pull-up
    funPinMode(BTN1, GPIO_CNF_IN_PUPD);
    funDigitalWrite(BTN1, FUN_HIGH);
    funPinMode(BTN2, GPIO_CNF_IN_PUPD);
    funDigitalWrite(BTN2, FUN_HIGH);

    Interrupt_Btn_Configure();
    Interrupt_Btn_Enable();


    // Turn off all LEDs initially (write HIGH for active-low)
    allLedsOff();
    const uint16_t LOOP_DELAY_MS = 50; // Poll buttons every 50ms

    // mode_ctf_main();
    ModeBlinky_Setup();
    while (1)
    {
        // // Update button states
        // uint8_t btn1_event = updateButton(BTN1, &btn1, LOOP_DELAY_MS);
        // uint8_t btn2_event = updateButton(BTN2, &btn2, LOOP_DELAY_MS);

        if (currentMode == MODE_BLINK && blinkingEnabled) {
            // funDigitalWrite(LED_PURPLE, LED_ON);
            runBlinkPattern(currentBlinkPattern);
        }
        else if (currentMode == MODE_CTF) {
            mode_ctf_main();
        }
        
    }
}



void Interrupt_BtnLeft(){
    currentMode = (currentMode == MODE_BLINK) ? MODE_CTF : MODE_BLINK;

    gpioLedInit();
    flashAllLeds();
    switch (currentMode) {
        case MODE_BLINK:
            ModeBlinky_Setup();
            break;
        default:
            gpioLedInit();
            break;
    }
}

void Interrupt_BtnRight(){
    // Handle BTN2 events - Only in BLINK mode
    if (currentMode == MODE_BLINK) {
        ModeBlinky_changeMode();
    }
}
