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
