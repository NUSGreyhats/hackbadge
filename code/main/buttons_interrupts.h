#include "ch32fun.h"

void Interrupt_BtnLeft();
void Interrupt_BtnRight();

void Interrupt_Btn_Configure(){
    RCC->APB2PCENR |= RCC_APB2Periph_AFIO;
    
    asm volatile(
#if __GNUC__ > 10
		".option arch, +zicsr\n"
#endif
 		"addi t1, x0, 3\n"
		"csrrw x0, 0x804, t1\n"
		 : : :  "t1" );

	// Configure the IO as an interrupt.
	AFIO->EXTICR = AFIO_EXTICR1_EXTI2_PC | AFIO_EXTICR1_EXTI0_PD;
	EXTI->INTENR = EXTI_INTENR_MR2 | EXTI_INTENR_MR0; // Enable EXT0

	// EXTI->RTENR  = EXTI_RTENR_TR2  | EXTI_RTENR_TR0;  // Rising edge trigger
	// EXTI->FTENR  = EXTI_FTENR_TR2  | EXTI_FTENR_TR0;  // Falling edge trigger

	EXTI->RTENR  = EXTI_RTENR_TR0;  // Rising edge trigger
	EXTI->FTENR  = EXTI_FTENR_TR2;  // Falling edge trigger
}

void Interrupt_Btn_Enable(){
    // enable interrupt
	NVIC_EnableIRQ( EXTI7_0_IRQn );
}

void Interrupt_Btn_Disable(){
	NVIC_DisableIRQ( EXTI7_0_IRQn );
}

void EXTI7_0_IRQHandler( void ) __attribute__((interrupt));
void EXTI7_0_IRQHandler( void ) 
{
    if (EXTI->INTFR & EXTI_Line2) { // Left Button (Line 2)
        Interrupt_BtnLeft();
	    EXTI->INTFR = EXTI_Line2; // Acknowledge the interrupt
        return;
    }

    // Right Button (Line 0)
    Interrupt_BtnRight();
    EXTI->INTFR = EXTI_Line0; // Acknowledge the interrupt   
}
