#include "pinout.h"
#include <stdio.h>

#define MODE_CTF_KEYLEN 12
volatile char mode_ctf_btn_buffer[MODE_CTF_KEYLEN];
void mode_ctf_btn_buffer_shift(char new_val){ // new bit at the 0th position
    for(int i=MODE_CTF_KEYLEN-1; i>=0; i--){
        mode_ctf_btn_buffer[i+1] = mode_ctf_btn_buffer[i];
    }
    mode_ctf_btn_buffer[0] = new_val;
} 

int mode_ctf_btn_buffer_map(){ // bit representation of buffer into decimal
    int val = 0;
    for (int i=0; i<MODE_CTF_KEYLEN; i++) {
        if (mode_ctf_btn_buffer == -1) { return -1; } // invalid
        val += (mode_ctf_btn_buffer[i]) << i;
    }
    return val;
} 

int mode_ctf_check_and_unlock(){
    int val = mode_ctf_btn_buffer_map();
    debug_print("%d: %d %d %d\n", val, mode_ctf_btn_buffer[0], mode_ctf_btn_buffer[1], mode_ctf_btn_buffer[2]);

    // badge state looks like blue green yellow orange red purple
    if (val == 0b110010101001){
        global_badge_state.unlock_status = 0xffffffff; // unlock all
        BadgeState_Write(global_badge_state);
        return 1;
    } 
    if (val == 0b111111111111){ // default no workshop mode
        global_badge_state.unlock_status = 0b101000; // 0b101000 yellow and blue
        BadgeState_Write(global_badge_state);
        return 1;
    }
    if (val == 0b0) { // hold down left button to go to mic
        return 1;
    }

    // Actual colour unlocks
    if (val == 0b010110101101) { 
        global_badge_state.unlock_status |= 0b010000; // green
        BadgeState_Write(global_badge_state);
        return 1;
    }
    if (val == 0b010011001101) {
        global_badge_state.unlock_status |= 0b000100; // orange
        BadgeState_Write(global_badge_state);
        return 1;
    }
    if (val == 0b101101001100) {
        global_badge_state.unlock_status |= 0b000010; // red
        BadgeState_Write(global_badge_state);
        return 1;
    }
    if (val == 0b111101000100) {
        global_badge_state.unlock_status |= 0b000001; // purple
        BadgeState_Write(global_badge_state);
        return 1;
    }

    // Erasing colours
    if (val == 0b111111111110) {
        global_badge_state.unlock_status &= ~(0b100000); // blue
        BadgeState_Write(global_badge_state);
        return 1;
    }
    if (val == 0b111111111101) {
        global_badge_state.unlock_status &= ~(0b010000); // green
        BadgeState_Write(global_badge_state);
        return 1;
    }
    if (val == 0b111111111011) {
        global_badge_state.unlock_status &= ~(0b001000); // yellow
        BadgeState_Write(global_badge_state);
        return 1;
    }
    if (val == 0b111111110111) {
        global_badge_state.unlock_status &= ~(0b000100); // orange
        BadgeState_Write(global_badge_state);
        return 1;
    }
    if (val == 0b111111101111) {
        global_badge_state.unlock_status &= ~(0b000010); // red
        BadgeState_Write(global_badge_state);
        return 1;
    }
    if (val == 0b111111011111) {
        global_badge_state.unlock_status &= ~(0b000001); // purple
        BadgeState_Write(global_badge_state);
        return 1;
    }
    if (val == 0b111111001111) {
        global_badge_state.unlock_status |= 0b1000000; // Extra 1
        BadgeState_Write(global_badge_state);
        return 1;
    }
    if (val == 0b11011100111) {
        global_badge_state.unlock_status |= 0b10000000; // Extra 2
        BadgeState_Write(global_badge_state);
        return 1;
    }

    return 0;
} 

int mode_ctf_led_ticks;
void mode_ctf_led(int yellow, int blue, int ms) {
    mode_ctf_led_ticks = SysTick->CNT;
    while (SysTick->CNT - mode_ctf_led_ticks < Ticks_from_Ms(ms)) {
        LED_PWM_Pass(16, 0, 0, yellow, 0, blue);
    }
}
void mode_ctf_main(){
    Interrupt_Btn_Disable();
    led_pwm_gpio_init();

    // Buffer, 8,8,8...
    memset(mode_ctf_btn_buffer, 8, MODE_CTF_KEYLEN);

    // funDigitalWrite(LED_YELLOW, LED_ON);
    while (1)
    {
        LED_PWM_Pass(16, 0, 0, 0, 0, 0);
        // Update button states
        uint8_t btnl_event = readButton(BTN_LEFT);
        uint8_t btnr_event = readButton(BTN_RIGHT);

        // Handle BTN1 events - Mode switch, left is 0 right is 1
        if (btnl_event == 1) {
            mode_ctf_btn_buffer_shift(0);
            if (mode_ctf_check_and_unlock()){ break; }
            mode_ctf_led(64, 0, 500); // yellow for 500ms, then 200ms delay
            mode_ctf_led(0, 0, 200);
        }

        if (btnr_event == 1) {
            funDigitalWrite(LED_ORANGE, LED_ON);
            mode_ctf_btn_buffer_shift(1);
            if (mode_ctf_check_and_unlock()){ break; }
            mode_ctf_led(0, 64, 500); // blue for 500ms, then 200ms delay
            mode_ctf_led(0,  0, 200);
        }
    }

    
    Interrupt_Btn_Enable();
    LED_PWM_Clear();
    blinky_fade_clear();
    Delay_Ms(1000);
    gpioLedInit();
    flashAllLeds();
    Delay_Ms(1000);
    currentMode = MODE_BLINK;
    ModeBlinky_changeMode();
}