/*
    Reference
     - https://github.com/cnlohr/ch32fun/blob/b32da76d825c12db6ad9a07961ed7d5eaa34cd3f/examples/exti_pin_change_isr/exti_pin_change_isr.c
 */

#include "ch32fun.h"
#include <stdio.h>


// LED pins for raw value display
#define LED_PURPLE   PD3
#define LED_BLUE     PC4
#define LED_GREEN    PC5
#define LED_YELLOW   PC6
#define LED_ORANGE   PC7
#define LED_RED      PD2

#define BTN_RIGHT    PD0
#define BTN_LEFT     PC2

void Interrupt_BtnLeft(){
    funDigitalWrite( LED_BLUE,    FUN_HIGH );
}

void Interrupt_BtnRight(){
    funDigitalWrite( LED_BLUE,    FUN_LOW );
}

#include "buttons_interrupts.h"

int main(void)
{
    SystemInit();

    funGpioInitAll();
    funPinMode(LED_PURPLE, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);
    funPinMode(LED_BLUE,   GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);
    funPinMode(LED_YELLOW, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);
    funPinMode(LED_ORANGE, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);
    funPinMode(LED_GREEN,  GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);
    funPinMode(LED_RED,    GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);

    // Clear LEDs
    funDigitalWrite(LED_ORANGE, FUN_HIGH);
    funDigitalWrite(LED_YELLOW, FUN_HIGH);
    funDigitalWrite(LED_BLUE, FUN_HIGH);
    funDigitalWrite(LED_PURPLE, FUN_HIGH);
    funDigitalWrite(LED_GREEN, FUN_HIGH);
    funDigitalWrite(LED_RED, FUN_HIGH);

	RCC->APB2PCENR = RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO;

    // Button Interrupt Trigger
	funPinMode( BTN_RIGHT, GPIO_CFGLR_IN_FLOAT );
	funPinMode( BTN_LEFT, GPIO_CFGLR_IN_FLOAT );

    Interrupt_Btn_Configure();
    Interrupt_Btn_Enable();
	

	while(1)
	{
		//GPIOC->BSHR = 1;
		//if( ((uint32_t*)count++) )
		//	GPIOC->BSHR = (1<<16);
		//if( count == 100 ) count = 0;
		asm volatile( "nop" );
	}
}