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

// LED pins for raw value display
#define LED_PURPLE   PC2
#define LED_BLUE     PC4
#define LED_YELLOW   PC6
#define LED_ORANGE   PC7

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

    // Sampling time: 241 cycles (maximum stability)
    ADC1->SAMPTR2 = (7 << 0);

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

void show_voltage_on_leds(uint16_t adc_val)
{
    // Turn off all LEDs
    funDigitalWrite(LED_ORANGE, FUN_HIGH);
    funDigitalWrite(LED_YELLOW, FUN_HIGH);
    funDigitalWrite(LED_BLUE, FUN_HIGH);
    funDigitalWrite(LED_PURPLE, FUN_HIGH);

    // Show voltage ranges on LEDs
    // 0-255 (0-0.82V) = Orange
    // 256-511 (0.82-1.65V) = Yellow
    // 512-767 (1.65-2.47V) = Blue
    // 768-1023 (2.47-3.3V) = Purple

    if (adc_val < 256) {
        funDigitalWrite(LED_ORANGE, FUN_LOW);
    }
    else if (adc_val < 512) {
        funDigitalWrite(LED_YELLOW, FUN_LOW);
    }
    else if (adc_val < 768) {
        funDigitalWrite(LED_BLUE, FUN_LOW);
    }
    else {
        funDigitalWrite(LED_PURPLE, FUN_LOW);
    }
}

int main(void)
{
    SystemInit();

    // Wait for debug printf
    while (!DebugPrintfBufferFree());
    Delay_Ms(100);

    printf("\r\n\r\n=== RAW ADC MONITOR ===\n");
    printf("No baseline tracking, no deviation calc\n");
    printf("Pure voltage readings from PA2\n\n");

    // Init ADC with pure analog input
    printf("Configuring PA2 as pure analog (no pull-up/down)...");
    adc_init_pure_analog();
    printf("done.\n\n");

    // Setup LEDs
    funGpioInitAll();
    funPinMode(LED_PURPLE, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);
    funPinMode(LED_BLUE, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);
    funPinMode(LED_YELLOW, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);
    funPinMode(LED_ORANGE, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);

    printf("Sample | ADC Value | Millivolts | LED\n");
    printf("-------|-----------|------------|-----\n");

    uint32_t sample_num = 0;
    uint16_t min_seen = 1023;
    uint16_t max_seen = 0;
    uint32_t sum = 0;

    while (1)
    {
        // Read raw ADC value
        uint16_t raw_adc = adc_buffer[0];

        // Convert to millivolts
        uint16_t mv = (raw_adc * 3300) / 1024;

        // Track statistics
        if (raw_adc < min_seen) min_seen = raw_adc;
        if (raw_adc > max_seen) max_seen = raw_adc;
        sum += raw_adc;
        sample_num++;

        // Determine which LED is lit
        char led_indicator;
        if (raw_adc < 256) led_indicator = 'O';  // Orange
        else if (raw_adc < 512) led_indicator = 'Y';  // Yellow
        else if (raw_adc < 768) led_indicator = 'B';  // Blue
        else led_indicator = 'P';  // Purple

        // Update LED
        show_voltage_on_leds(raw_adc);

        // Print every 50ms
        printf("%6lu | %4d      | %4d       | %c\n",
            sample_num, raw_adc, mv, led_indicator);

        // Print statistics every 10 seconds (200 samples)
        if (sample_num % 200 == 0) {
            uint16_t avg = sum / 200;
            uint16_t avg_mv = (avg * 3300) / 1024;
            uint16_t min_mv = (min_seen * 3300) / 1024;
            uint16_t max_mv = (max_seen * 3300) / 1024;
            uint16_t pk_pk = max_seen - min_seen;
            uint16_t pk_pk_mv = (pk_pk * 3300) / 1024;

            printf("\n--- Statistics (last 10s) ---\n");
            printf("Average: %4d (%4d mV)\n", avg, avg_mv);
            printf("Min:     %4d (%4d mV)\n", min_seen, min_mv);
            printf("Max:     %4d (%4d mV)\n", max_seen, max_mv);
            printf("Peak-Peak: %4d (%4d mV)\n\n", pk_pk, pk_pk_mv);

            // Reset stats
            min_seen = 1023;
            max_seen = 0;
            sum = 0;
        }

        Delay_Ms(50);
    }
}