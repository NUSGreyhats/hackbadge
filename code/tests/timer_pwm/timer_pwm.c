/*
 * Example for using Advanced Control Timer (TIM1) for PWM generation
 * 03-28-2023 E. Brombaugh
 */

 #include "ch32fun.h"
 #include <stdio.h>
 
// LED pins for raw value display
#define LED_PURPLE   PD3 //        T1CH4_1                          T2CH2, T2CH2_2/
#define LED_RED      PD2 //                   T1CH1_2   T1CH2N_3,          T2CH3_1
#define LED_ORANGE   PC7 //        T1CH2_1              T1CH2_3            T2CH2_3  
#define LED_YELLOW   PC6 //        T1CH1_1,             T1CH3N_3 
#define LED_GREEN    PC5 //                             T1CH3_3      T1ETR, T2CH1ETR(1)_1,
#define LED_BLUE     PC4 // T1CH4, T1CH2N_1 / T1CH4_2 / T1CH1_3

#define funPinModeClear(pin, no) { GpioOf(pin)->CFGLR &= ~((GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*no)); }

#define BTN_RIGHT    PD0
#define BTN_LEFT     PC2

// Mapping Timer 1
void TIM1Remap(void) {
	if(AFIO->PCFR1 & AFIO_PCFR1_TIM1_REMAP_FULLREMAP) {
		AFIO->PCFR1 &= ~AFIO_PCFR1_TIM1_REMAP_FULLREMAP;   //clear remapping bits
		printf("Standard Mapping!\r\n");
	}
	else {
		AFIO->PCFR1 |= AFIO_PCFR1_TIM1_REMAP_FULLREMAP; //set fullremap mode
		printf("Full Remapping!\r\n");
	}
};

// Mapping Timer 2
void TIM2Remap(void) {
	if(AFIO->PCFR1 & AFIO_PCFR1_TIM2_REMAP_FULLREMAP) {
		AFIO->PCFR1 &= ~AFIO_PCFR1_TIM2_REMAP_FULLREMAP;   //clear remapping bits
		printf("Standard Mapping!\r\n");
	}
	else {
		AFIO->PCFR1 |= AFIO_PCFR1_TIM2_REMAP_FULLREMAP; //set fullremap mode
		printf("Full Remapping!\r\n");
	}
};
 /*
  * initialize TIM1 for PWM
  */
void t1pwm_init( void )
{
    // Enable GPIOC, GPIOD and TIM1
    RCC->APB2PCENR |= 	RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOC |
                         RCC_APB2Periph_TIM1;
    
    // PC4 is T1CH4, 10MHz Output alt func, push-pull
    funPinMode( PC7, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF ); 

    funPinMode( PC6, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF );
    funPinMode( PC5, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF );
    funPinMode( PC4, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF );
    funPinMode( PD2, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF );

    // Remap Register
    // AFIO->PCFR1 |= AFIO_PCFR1_TIM1_REMAP_FULLREMAP;
    // AFIO->PCFR1 |= AFIO_PCFR1_TIM1_REMAP_FULLREMAP;


    // Reset TIM1 to init all regs
    RCC->APB2PRSTR |= RCC_APB2Periph_TIM1;
    RCC->APB2PRSTR &= ~RCC_APB2Periph_TIM1;
     
    // CTLR1: default is up, events generated, edge align
    // SMCFGR: default clk input is CK_INT

    TIM1->PSC = 0x0000;     // Prescaler 
    TIM1->ATRLR = 255;      // Auto Reload - sets period
    TIM1->SWEVGR |= TIM_UG; // Reload immediately
     
    // Enable CH1N output, positive pol
    //  TIM1->CCER |= TIM_CC1NE | TIM_CC1NP;
     
     // Enable CH4 output, positive pol
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
    // TIM1->CH1CVR = 128;
    // TIM1->CH2CVR = 128;
    // TIM1->CH3CVR = 128;
    // TIM1->CH4CVR = 128;
    TIM1->CH1CVR = 0;
    TIM1->CH2CVR = 0;
    TIM1->CH3CVR = 0;
    TIM1->CH4CVR = 0;
     
     
    // Enable TIM1 outputs
    TIM1->BDTR |= TIM_MOE;
     
    // Enable TIM1
    TIM1->CTLR1 |= TIM_CEN;
}

/*
* set timer channel PW
*/
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

/*
 * initialize TIM2 for PWM
 */
void t2pwm_init( void )
{
	// Enable GPIOD and TIM2
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD;
	RCC->APB1PCENR |= RCC_APB1Periph_TIM2;

	#ifdef TIM2_EN_CH3
	// If using T2CH3 must also enable GPIOC
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOC;
	#endif

	// PD4 is T2CH1, 10MHz Output alt func, push-pull
	
	// PD3 is T2CH2, 10MHz Output alt func, push-pull
    funPinMode( PD3, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF );

	// Reset TIM2 to init all regs
	RCC->APB1PRSTR |= RCC_APB1Periph_TIM2;
	RCC->APB1PRSTR &= ~RCC_APB1Periph_TIM2;
	
	// SMCFGR: default clk input is CK_INT
	// set TIM2 clock prescaler divider 
	TIM2->PSC = 0x0000;
	// set PWM total cycle width
	TIM2->ATRLR = 255;
	
	// for channel 1 and 2, let CCxS stay 00 (output), set OCxM to 110 (PWM I)
	// enabling preload causes the new pulse width in compare capture register only to come into effect when UG bit in SWEVGR is set (= initiate update) (auto-clears)
	TIM2->CHCTLR1 |= TIM_OC1M_2 | TIM_OC1M_1 | TIM_OC1PE | TIM_OC2M_2 | TIM_OC2M_1 | TIM_OC2PE;
    TIM2->CHCTLR2 |= TIM_OC3M_2 | TIM_OC3M_1 | TIM_OC3PE;
	TIM2->CHCTLR2 |= TIM_OC4M_2 | TIM_OC4M_1 | TIM_OC4PE;
	
    // CTLR1: default is up, events generated, edge align
	// enable auto-reload of preload
	TIM2->CTLR1 |= TIM_ARPE;

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


/*
 * set timer channel PW
 */
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

  

void clear(int val){
    for (int i=0; i<4; i++){
        t1pwm_setpw(i, val); 
        t2pwm_setpw(i, val); 
    }
}

 
// LED pins for raw value display
#define LED_PURPLE   PD3 //        T1CH4_1                          T2CH2, T2CH2_2/
#define LED_RED      PD2 //                   T1CH1_2   T1CH2N_3,          T2CH3_1
#define LED_ORANGE   PC7 //        T1CH2_1              T1CH2_3            T2CH2_3  
#define LED_YELLOW   PC6 //        T1CH1_1,             T1CH3N_3 
#define LED_GREEN    PC5 //                             T1CH3_3      T1ETR, T2CH1ETR(1)_1,
#define LED_BLUE     PC4 // T1CH4, T1CH2N_1 / T1CH4_2 / T1CH1_3

void displayLEDPWM(int purple, int red, int orange, int yellow, int green, int blue){
    // Purple
    // Delay_Ms( 1000 );
    clear(0);
    AFIO->PCFR1 &= ~AFIO_PCFR1_TIM1_REMAP_FULLREMAP; 
    funPinMode( LED_BLUE, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF );
    funPinMode(PD2, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF );
    funPinMode(PC7, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF );
    funPinMode(PC6, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF );

    t1pwm_setpw(0, red); 
    t2pwm_setpw(1, purple); 
    // t1pwm_setpw(3, blue);


    // Change
    // Delay_Ms( 1000 );
    clear(0);
    AFIO->PCFR1 |=  AFIO_PCFR1_TIM1_REMAP_PARTIALREMAP1;
    funPinModeClear(PC4, 4);
    funPinMode(PD2, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF );
    funPinMode(PC7, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF );

    t1pwm_setpw(0, yellow);
    t1pwm_setpw(1, orange); // linked to blue?
    
    
    // Delay_Ms( 1000 );
    // clear(0);
    // funPinMode( LED_BLUE, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF );
    // AFIO->PCFR1 &= ~AFIO_PCFR1_TIM1_REMAP_PARTIALREMAP1;
    // AFIO->PCFR1 |=  AFIO_PCFR1_TIM1_REMAP_PARTIALREMAP2;
    // t1pwm_setpw(0, red); 
    // t2pwm_setpw(1, purple); 
    // t1pwm_setpw(3, blue);

    // Delay_Ms( 1000 );
    clear(0);
    AFIO->PCFR1 |=  AFIO_PCFR1_TIM1_REMAP_PARTIALREMAP2;
    funPinMode( LED_BLUE, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF );
    funPinModeClear(PD2, 2);
    funPinModeClear(PC7, 7);
    funPinModeClear(PC6, 6);
    // t1pwm_setpw(0, green);
    // t1pwm_setpw(1, green);
    t1pwm_setpw(2, green);
    // t1pwm_setpw(3, green);

}


 /*
  * entry
  */
 int main()
 {
     uint32_t count = 0;
     
     SystemInit();
     Delay_Ms( 100 );

     t1pwm_init();
     t2pwm_init();

    count = 8;

    uint32_t ticks = SysTick->CNT;
    while(1)
     {
        // displayLEDPWM(255, count, 255, count, 128, count);
        // displayLEDPWM(64, count, 64, count, 16, count);
        displayLEDPWM(0,0,0,0,0,0);
        if (SysTick->CNT - ticks > Ticks_from_Ms(2000)) {
            break;
        }
     }

    ticks = SysTick->CNT;
    int counter = 0;
    int og[] = {64, 8, 64, 8, 16, 8};
    int values[] = {0, 0, 0, 0, 0, 0};
    while(1)
     {
        // displayLEDPWM(255, count, 255, count, 128, count);
        displayLEDPWM(values[0], values[1], values[2], values[3], values[4], values[5]);

        if (SysTick->CNT - ticks > Ticks_from_Ms(1000)) {
            values[counter] = 0;
            counter = (counter + 1) % 6;
            values[counter] = 16; //og[counter];
            ticks = SysTick->CNT;
        }
     }



 }