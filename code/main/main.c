#include "ch32fun.h"
#include <stdio.h>
#include <string.h>

void debug_print(const char* format, ...) {
    #if FUNCONF_USE_DEBUGPRINTF
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    #endif
}

#include "pinout.h"  
#include "led_timers.h"
#include "led_gpio.h"
#include "buttons.h"
#include "buttons_interrupts.h"
#include "badge_state.h"

// Button pin definitions
#define BTN1   BTN_LEFT
#define BTN2   BTN_RIGHT

// LEDs are active LOW (GPIO LOW = LED ON)
#define LED_ON   FUN_LOW
#define LED_OFF  FUN_HIGH

// Modes
typedef enum {
    MODE_BLINK = 0,
    MODE_MIC = 1,
    MODE_CTF = 2
} SystemMode;
static SystemMode currentMode = MODE_BLINK;

volatile badge_state global_badge_state;

// Flash all LEDs for 1 second (mode change indicator)
void flashAllLeds()
{
    //funDigitalWrite(LED_PURPLE, LED_ON);
    //funDigitalWrite(LED_BLUE, LED_ON);
    //funDigitalWrite(LED_YELLOW, LED_ON);
    lightUpLed("blue", 10, 1000);
    Delay_Ms(1000);
    allLedsOff();
}

#include "mode/mode_blinky.h"
#include "mode/mode_ctf.h"
#include "mode/mode_mic.h"

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

    global_badge_state = BadgeState_Read();
    // global_badge_state.unlock_status = 0xffffffff;
    // global_badge_state.unlock_status = 0x2c;
    // BadgeState_SetUnlocked

    // Turn off all LEDs initially (write HIGH for active-low)
    // xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    const uint16_t LOOP_DELAY_MS = 50; // Poll buttons every 50ms

	// enable power interface module clock
	RCC->APB1PCENR |= RCC_APB1Periph_PWR;
	// enable low speed oscillator (LSI)
	RCC->RSTSCKR |= RCC_LSION;
	while ((RCC->RSTSCKR & RCC_LSIRDY) == 0) {}
	// enable AutoWakeUp event
	EXTI->EVENR |= EXTI_Line9;
	EXTI->FTENR |= EXTI_Line9;
	// configure AWU prescaler
	PWR->AWUPSC |= PWR_AWU_Prescaler_4096;
	// configure AWU window comparison value
	PWR->AWUWR &= ~0x3f;
	PWR->AWUWR |= 63;
	// enable AWU
	PWR->AWUCSR |= (1 << 1);
	// select standby on power-down
	PWR->CTLR |= PWR_CTLR_PDDS;
	// peripheral interrupt controller send to deep sleep
	PFIC->SCTLR |= (1 << 2);

    // mode_ctf_main();
    debug_print("Welcome to Hackbadge\n");
    ModeBlinky_Setup();

    
    __WFE();
    // SystemInit();

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
        else if (currentMode == MODE_MIC) {
            mode_mic_main();
        }
        
    }
}

void Interrupt_BtnLeft(){

    // Debounce (kinda)
    int wait_ticks = SysTick->CNT;
    while (SysTick->CNT - wait_ticks < Ticks_from_Ms(200)) {}
    while (funDigitalRead(BTN_LEFT) == 0) {} //  Holding down
    int long_press = SysTick->CNT - wait_ticks > Ticks_from_Ms(3000); // Hold for 3 seconds

    
    if (long_press){
        if (currentMode == MODE_BLINK) {currentMode = MODE_MIC;} 
        else {currentMode = MODE_BLINK;}
    } else {
        currentMode = MODE_CTF;
    }
    // currentMode = (SystemMode)((currentMode + 1) % 3);

    // LED_PWM_Clear();
    // gpioLedInit();
    // flashAllLeds();
    switch (currentMode) {
        case MODE_BLINK:
            ModeBlinky_Setup();
            break;
        case MODE_MIC:
            mode_mic_setup();
            break;
        default:
            gpioLedInit();
            break;
    }
}

void Interrupt_BtnRight(){
    // Debounce (kinda)
    int wait_ticks = SysTick->CNT;
    while (SysTick->CNT - wait_ticks < Ticks_from_Ms(200)) {}
    
    // Handle BTN2 events - Only in BLINK mode
    if (currentMode == MODE_BLINK) {
        ModeBlinky_changeMode();
    }
}
