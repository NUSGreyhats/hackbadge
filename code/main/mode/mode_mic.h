/*
 * RAW ADC Monitor - Zero Processing
 * Shows direct ADC readings from PA2 with no baseline tracking,
 * no deviation calculation, just pure voltage measurements
 *
 * This reveals:
 * - Actual DC bias voltage on PA2
 * - Whether internal pull-up/pull-down affects readings
 * - Raw AC + DC signal from microphone
 */

#include "ch32fun.h"
#include <stdio.h>

#define ADC_NUMCHLS 1
volatile uint16_t adc_buffer[ADC_NUMCHLS];

void adc_init_pure_analog(void)
{
    // ADC Clock: 24MHz / 2 = 12MHz
    RCC->CFGR0 &= ~(0x1F << 11);

    // Enable clocks
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1;

    // CRITICAL: Configure PA2 as PURE ANALOG INPUT
    // This disables ALL internal pull-ups/pull-downs
    // CNF=00 (Analog), MODE=00 (Input)
    GPIOA->CFGLR &= ~(0xf << (4 * 2));
    // Do NOT set ODR high - leave PA2 completely floating/analog

    // Reset ADC
    RCC->APB2PRSTR |= RCC_APB2Periph_ADC1;
    RCC->APB2PRSTR &= ~RCC_APB2Periph_ADC1;

    // Single channel: Channel 0 (PA2)
    ADC1->RSQR1 = 0 << 20;  // 1 conversion
    ADC1->RSQR2 = 0;
    ADC1->RSQR3 = 0;  // Channel 0

    // Sampling time: 61 cycles (good for voice)
    ADC1->SAMPTR2 = (5 << 0);

    // Enable ADC
    ADC1->CTLR2 |= ADC_ADON;

    // Calibration
    ADC1->CTLR2 |= ADC_RSTCAL;
    while (ADC1->CTLR2 & ADC_RSTCAL);
    ADC1->CTLR2 |= ADC_CAL;
    while (ADC1->CTLR2 & ADC_CAL);

    // DMA setup
    RCC->AHBPCENR |= RCC_AHBPeriph_DMA1;
    DMA1_Channel1->PADDR = (uint32_t)&ADC1->RDATAR;
    DMA1_Channel1->MADDR = (uint32_t)adc_buffer;
    DMA1_Channel1->CNTR = ADC_NUMCHLS;
    DMA1_Channel1->CFGR =
        DMA_M2M_Disable |
        DMA_Priority_VeryHigh |
        DMA_MemoryDataSize_HalfWord |
        DMA_PeripheralDataSize_HalfWord |
        DMA_MemoryInc_Enable |
        DMA_Mode_Circular |
        DMA_DIR_PeripheralSRC;
    DMA1_Channel1->CFGR |= DMA_CFGR1_EN;

    // Single channel - no SCAN
    ADC1->CTLR1 &= ~ADC_SCAN;

    // Continuous conversion
    ADC1->CTLR2 &= ~(0b111 << 17);
    ADC1->CTLR2 |= ADC_CONT | ADC_DMA;
    ADC1->CTLR2 |= ADC_SWSTART;
}

void show_voltage_on_leds(uint16_t adc_val) {
    // Clear all LED values
    blinky_fade_clear();

    // Progressive LED display based on voltage
    // Array is: purple[0], red[1], orange[2], yellow[3], green[4], blue[5]

    if (adc_val >= 5) {
        values[5] = max_brightness[5];  // Blue - baseline
    }
    if (adc_val >= 7) {
        values[4] = max_brightness[4];  // Green
    }
    if (adc_val >= 10) {
        values[3] = max_brightness[3];  // Yellow
    }
    if (adc_val >= 12) {
        values[2] = max_brightness[2];  // Orange
    }
    if (adc_val >= 15) {
        values[1] = max_brightness[1];  // Red
    }
    if (adc_val >= 25) {
        values[0] = max_brightness[0];  // Purple
    }

    // Update LEDs
    LED_PWM_Pass(values[0], values[1], values[2], values[3], values[4], values[5]);
}


// Mic debugging variables
static uint32_t sample_num = 0;
static uint16_t min_value = 1023;
static uint16_t max_value = 0;

void mode_mic_setup() {
    LED_PWM_Clear();
    LED_PWM_Init();
    blinky_fade_clear();
    ticks = SysTick->CNT; 
    Delay_Ms(100);

    debug_print("\r\n\r\n=== RAW ADC MONITOR ===\n");
    debug_print("No baseline tracking, no deviation calc\n");
    debug_print("Pure voltage readings from PA2\n\n");

    // Init ADC with pure analog input
    debug_print("Configuring PA2 as pure analog (no pull-up/down)...");
    adc_init_pure_analog();
    debug_print("done.\n\n");

    // Reset state
    sample_num = 0;
    min_value = 1023;
    max_value = 0;
}


void mode_mic_main() {
    LED_PWM_Pass(values[0], values[1], values[2], values[3], values[4], values[5]);

    if (SysTick->CNT - ticks < Ticks_from_Us(500)) { // Allows me to keep calling LED PWM
        return;
    }
    ticks = SysTick->CNT;

    // Read raw ADC value
    uint16_t raw_adc = adc_buffer[0];

    // Update LED
    // show_voltage_on_leds(raw_adc);

    // Convert to millivolts
    // uint16_t mv = (raw_adc * 5000) / 1024;

    // Track statistics
    // if (raw_adc < min_seen) min_seen = raw_adc;
    // if (raw_adc > max_seen) max_seen = raw_adc;
    // sum += raw_adc;
    // sample_num++;

    // // Determine which LED is lit
    // char* led_str;
    // if (raw_adc < 200) {
    //     led_str = "-";
    // } else if (raw_adc < 550) {
    //     led_str = "B";
    // } else if (raw_adc < 600) {
    //     led_str = "BG";
    // } else if (raw_adc < 650) {
    //     led_str = "BGY";
    // } else if (raw_adc < 700) {
    //     led_str = "BGYO";
    // } else if (raw_adc < 750) {
    //     led_str = "BGYOR";
    // } else {
    //     led_str = "BGYORP";
    // }

    // // Print every 50ms
    // debug_print("%6lu | %4d      | %4d       | %s\n",
    //     sample_num, raw_adc, mv, led_str);

    // // Print statistics every 10 seconds (200 samples)
    // if (sample_num % 200 == 0) {
    //     uint16_t avg = sum / 200;
    //     uint16_t avg_mv = (avg * 5000) / 1024;
    //     uint16_t min_mv = (min_seen * 5000) / 1024;
    //     uint16_t max_mv = (max_seen * 5000) / 1024;
    //     uint16_t pk_pk = max_seen - min_seen;
    //     uint16_t pk_pk_mv = (pk_pk * 5000) / 1024;

    //     debug_print("\n--- Statistics (last 10s) ---\n");
    //     debug_print("Average: %4d (%4d mV)\n", avg, avg_mv);
    //     debug_print("Min:     %4d (%4d mV)\n", min_seen, min_mv);
    //     debug_print("Max:     %4d (%4d mV)\n", max_seen, max_mv);
    //     debug_print("Peak-Peak: %4d (%4d mV)\n\n", pk_pk, pk_pk_mv);

    //     // Reset stats
    //     min_seen = 1023;
    //     max_seen = 0;
    //     sum = 0;
    // }
    
    // Continuously update min/max from ADC samples
    if (raw_adc < min_value) min_value = raw_adc;
    if (raw_adc > max_value) max_value = raw_adc;
    sample_num++;
    
    if (sample_num == 200) {
        // Print & update every 100ms
        uint16_t peak_to_peak = max_value - min_value;
        uint16_t center = (max_value + min_value) / 2;
        
        debug_print("P2P: %4d | Ctr: %4d\n", peak_to_peak, center);
        
        show_voltage_on_leds(peak_to_peak);

        // Reset tracking
        min_value = 1023;
        max_value = 0;
        sample_num = 0;
    }
    return;
}

