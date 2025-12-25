#include "pinout.h"
#include <stdio.h>

#define MODE_CTF_KEYLEN 8
volatile char mode_ctf_btn_buffer[MODE_CTF_KEYLEN];
void mode_ctf_btn_buffer_shift(char new_val){
    for(int i=MODE_CTF_KEYLEN-1; i>=0; i--){
        mode_ctf_btn_buffer[i+1] = mode_ctf_btn_buffer[i];
    }
    mode_ctf_btn_buffer[0] = new_val;
    printf("%d %d %d\n", mode_ctf_btn_buffer[0], mode_ctf_btn_buffer[1], mode_ctf_btn_buffer[2]);
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

    if (mode_ctf_btn_buffer[0] == 0 && mode_ctf_btn_buffer[1] == 0 && mode_ctf_btn_buffer[2] == 1){
        global_badge_state.unlock_status = 0xffffffff; // unlock all
        // BadgeState_Write(global_badge_state);
        return 1;
    } 

    return 0;
} 

int mode_ctf_main(){
    Interrupt_Btn_Disable();
    gpioLedInit();

    // Buffer
    memset(mode_ctf_btn_buffer, 8, MODE_CTF_KEYLEN);

    funDigitalWrite(LED_YELLOW, LED_ON);
    while (1)
    {
        // // Update button states
        uint8_t btnl_event = readButton(BTN_LEFT);
        uint8_t btnr_event = readButton(BTN_RIGHT);

        // Handle BTN1 events - Mode switch
        if (btnl_event == 1) {
            funDigitalWrite(LED_RED, LED_ON);
            mode_ctf_btn_buffer_shift(0);
            if (mode_ctf_check_and_unlock()){ break; }
            Delay_Ms(1000);
            funDigitalWrite(LED_RED, LED_OFF);
        }

        if (btnr_event == 1) {
            funDigitalWrite(LED_ORANGE, LED_ON);
            mode_ctf_btn_buffer_shift(1);
            if (mode_ctf_check_and_unlock()){ break; }
            Delay_Ms(1000);
            funDigitalWrite(LED_ORANGE, LED_OFF);
        }
    }

    Interrupt_Btn_Enable();
    Delay_Ms(1000);
    currentMode = MODE_BLINK;
}