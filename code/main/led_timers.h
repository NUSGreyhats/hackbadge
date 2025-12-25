/*
 * Example for using Advanced Control Timer (TIM1) for PWM generation
 * 03-28-2023 E. Brombaugh
 */

#include "ch32fun.h"
#include <stdio.h>
#include "pinout.h"

// #define LED_USE_TIMER2 // For a brighter Purple

//// Timer Setup/ Wrapper
void led_pwm_gpio_init ( void ) {
    
    // Enable GPIOC, GPIOD and TIM1
    RCC->APB2PCENR |=  RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOC |
                       RCC_APB2Periph_TIM1;
                         
    // Timer 1
    funPinMode( PC7, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF ); 
    funPinMode( PC6, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF );
    funPinMode( PC5, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF );
    funPinMode( PC4, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF );
    funPinMode( PD2, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF );
    // Timer 1/2
    funPinMode( PD3, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF );
}

void t1pwm_init( void )
{
    RCC->APB2PCENR |=  RCC_APB2Periph_TIM1;
    // Reset TIM1 to init all regs
    RCC->APB2PRSTR |= RCC_APB2Periph_TIM1;
    RCC->APB2PRSTR &= ~RCC_APB2Periph_TIM1;
     
    // CTLR1: default is up, events generated, edge align
    // SMCFGR: default clk input is CK_INT
    TIM1->PSC = 0x0000;     // Prescaler 
    TIM1->ATRLR = 255;      // Auto Reload - sets period
    TIM1->SWEVGR |= TIM_UG; // Reload immediately
 
    TIM1->CCER |= TIM_CC1E | TIM_CC1P;
    TIM1->CCER |= TIM_CC2E | TIM_CC2P;
    TIM1->CCER |= TIM_CC3E | TIM_CC3P;
    TIM1->CCER |= TIM_CC4E | TIM_CC4P;
    
    // CH1 Mode is output, PWM1 (CC1S = 00, OC1M = 110)
    TIM1->CHCTLR1 |= TIM_OC1M_2 | TIM_OC1M_1;
    TIM1->CHCTLR1 |= TIM_OC2M_2 | TIM_OC2M_1;
    // CH2 Mode is output, PWM1 (CC1S = 00, OC1M = 110)
    TIM1->CHCTLR2 |= TIM_OC3M_2 | TIM_OC3M_1;
    TIM1->CHCTLR2 |= TIM_OC4M_2 | TIM_OC4M_1;
     
    // Set the Capture Compare Register value to 50% initially
    TIM1->CH1CVR = 0;
    TIM1->CH2CVR = 0;
    TIM1->CH3CVR = 0;
    TIM1->CH4CVR = 0;
     
    // Enable TIM1 outputs
    TIM1->BDTR |= TIM_MOE;
     
    // Enable TIM1
    TIM1->CTLR1 |= TIM_CEN;
}

void t1pwm_setpw(uint8_t chl, uint16_t width)
{
    switch(chl&3)
    {
        case 0: TIM1->CH1CVR = width; break;
        case 1: TIM1->CH2CVR = width; break;
        case 2: TIM1->CH3CVR = width; break;
        case 3: TIM1->CH4CVR = width; break;
    }
}
 
/*
* force output (used for testing / debug)
*/
void t1pwm_force(uint8_t chl, uint8_t val)
{
    uint16_t temp;
    
    chl &= 3;
    
    if(chl < 2)
    {
        temp = TIM1->CHCTLR1;
        temp &= ~(TIM_OC1M<<(8*chl));
        temp |= (TIM_OC1M_2 | (val?TIM_OC1M_0:0))<<(8*chl);
        TIM1->CHCTLR1 = temp;
    }
    else
    {
        chl &= 1;
        temp = TIM1->CHCTLR2;
        temp &= ~(TIM_OC1M<<(8*chl));
        temp |= (TIM_OC1M_2 | (val?TIM_OC1M_0:0))<<(8*chl);
        TIM1->CHCTLR2 = temp;
    }
}

// mask for the CCxP bits
// when set PWM outputs are held HIGH by default and pulled LOW
// when zero PWM outputs are held LOW by default and pulled HIGH
#define TIM2_DEFAULT 0xff
//#define TIM2_DEFAULT 0x00
void t2pwm_init( void )
{
	RCC->APB1PCENR |= RCC_APB1Periph_TIM2;

	RCC->APB1PRSTR |= RCC_APB1Periph_TIM2;
	RCC->APB1PRSTR &= ~RCC_APB1Periph_TIM2;
	
	// SMCFGR: default clk input is CK_INT
	TIM2->PSC = 0x0000; // set TIM2 clock prescaler divider 
	TIM2->ATRLR = 255; // set PWM total cycle width
	
	// for channel 1 and 2, let CCxS stay 00 (output), set OCxM to 110 (PWM I)
	// enabling preload causes the new pulse width in compare capture register only to come into effect when UG bit in SWEVGR is set (= initiate update) (auto-clears)
	TIM2->CHCTLR1 |= TIM_OC1M_2 | TIM_OC1M_1 | TIM_OC1PE | TIM_OC2M_2 | TIM_OC2M_1 | TIM_OC2PE;
    TIM2->CHCTLR2 |= TIM_OC3M_2 | TIM_OC3M_1 | TIM_OC3PE;
	TIM2->CHCTLR2 |= TIM_OC4M_2 | TIM_OC4M_1 | TIM_OC4PE;
	
    // CTLR1: default is up, events generated, edge align
	TIM2->CTLR1 |= TIM_ARPE; // enable auto-reload of preload

	// Enable Channel outputs, set default state (based on TIM2_DEFAULT)
	TIM2->CCER |= TIM_CC1E | (TIM_CC1P & TIM2_DEFAULT);
	TIM2->CCER |= TIM_CC2E | (TIM_CC2P & TIM2_DEFAULT);
    TIM2->CCER |= TIM_CC3E | (TIM_CC3P & TIM2_DEFAULT);
	TIM2->CCER |= TIM_CC4E | (TIM_CC4P & TIM2_DEFAULT);

    TIM2->CH1CVR = 0;
    TIM2->CH2CVR = 0;
    TIM2->CH3CVR = 0;
    TIM2->CH4CVR = 0;

	// initialize counter
	TIM2->SWEVGR |= TIM_UG;

	// Enable TIM2
	TIM2->CTLR1 |= TIM_CEN;
}

void t2pwm_setpw(uint8_t chl, uint16_t width)
{
	switch(chl&3)
	{
		case 0: TIM2->CH1CVR = width; break;
		case 1: TIM2->CH2CVR = width; break;
		case 2: TIM2->CH3CVR = width; break;
		case 3: TIM2->CH4CVR = width; break;
	}
	//TIM2->SWEVGR |= TIM_UG; // load new value in compare capture register
}

//// Application Functions ///////////////////////////////////////////////////////////////////////////////////////////////


void t1pwm_clear(int val){
    t1pwm_setpw(0, val); 
    t1pwm_setpw(1, val); 
    t1pwm_setpw(2, val); 
    t1pwm_setpw(3, val); 
}

 
// // LED pins for raw value display
// #define LED_PURPLE   PD3 //        T1CH4_1                          T2CH2, T2CH2_2/
// #define LED_RED      PD2 //                   T1CH1_2   T1CH2N_3,          T2CH3_1
// #define LED_ORANGE   PC7 //        T1CH2_1              T1CH2_3            T2CH2_3  
// #define LED_YELLOW   PC6 //        T1CH1_1,             T1CH3N_3 
// #define LED_GREEN    PC5 //                             T1CH3_3      T1ETR, T2CH1ETR(1)_1,
// #define LED_BLUE     PC4 // T1CH4, T1CH2N_1 / T1CH4_2 / T1CH1_3

void LED_PWM_Init() {
    led_pwm_gpio_init();
    t1pwm_init();
    #ifdef LED_USE_TIMER2
        t2pwm_init();
    #endif
}

void LED_PWM_Clear() {
    t1pwm_clear(0);
}

void LED_PWM_Pass(int purple, int red, int orange, int yellow, int green, int blue){
    // Purple
    // Delay_Ms( 1000 );
    t1pwm_clear(0);
    AFIO->PCFR1 &= ~AFIO_PCFR1_TIM1_REMAP_FULLREMAP; 
    funPinMode(PD2, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF );
    funPinMode(PC7, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF );
    funPinMode(PC6, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF );

    t1pwm_setpw(0, red); 
    #ifdef LED_USE_TIMER2
        t2pwm_setpw(1, purple); 
    #endif
    t1pwm_setpw(3, blue);


    // Delay_Ms( 1000 );
    t1pwm_clear(0);
    funPinModeClear(PC4, 4);
    AFIO->PCFR1 |=  AFIO_PCFR1_TIM1_REMAP_PARTIALREMAP1;

    t1pwm_setpw(0, yellow);
    t1pwm_setpw(1, orange); // linked to blue?
    #ifndef LED_USE_TIMER2
        t1pwm_setpw(3, purple); 
    #endif
    
    // Delay_Ms( 1000 );
    t1pwm_clear(0);
    funPinModeClear(PD2, 2);
    funPinModeClear(PC7, 7);
    funPinModeClear(PC6, 6);
    AFIO->PCFR1 |=  AFIO_PCFR1_TIM1_REMAP_PARTIALREMAP2;
    funPinMode( PC4, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF ); // BLUE
    t1pwm_setpw(2, green);

}

void LED_PWM_Test_Pass () {
    LED_PWM_Pass(64, 64, 64, 64, 8, 64);
}