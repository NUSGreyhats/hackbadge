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


void EXTI7_0_IRQHandler( void ) __attribute__((interrupt));
void EXTI7_0_IRQHandler( void ) 
{
	// Flash just a little blip.
	funDigitalWrite( LED_PURPLE, FUN_LOW );

	// Acknowledge the interrupt
	EXTI->INTFR = EXTI_Line0;
}


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
    
    funDigitalWrite(LED_ORANGE, FUN_LOW);

    // Button Interrupt Trigger
	funPinMode( BTN_RIGHT, GPIO_CFGLR_IN_FLOAT );
	RCC->APB2PCENR = RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO;

    	asm volatile(
#if __GNUC__ > 10
		".option arch, +zicsr\n"
#endif
 		"addi t1, x0, 3\n"
		"csrrw x0, 0x804, t1\n"
		 : : :  "t1" );

	// Configure the IO as an interrupt.
	AFIO->EXTICR = AFIO_EXTICR1_EXTI0_PD;
	EXTI->INTENR = EXTI_INTENR_MR0; // Enable EXT3
	EXTI->RTENR = EXTI_FTENR_TR0;  // Rising edge trigger

	// enable interrupt
	NVIC_EnableIRQ( EXTI7_0_IRQn );

	while(1)
	{
		//GPIOC->BSHR = 1;
		//if( ((uint32_t*)count++) )
		//	GPIOC->BSHR = (1<<16);
		//if( count == 100 ) count = 0;
		asm volatile( "nop" );
	}
}