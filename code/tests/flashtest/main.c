#include "ch32fun.h"
#include <stdio.h>

/// Flash Helpers ////////////////////////////////////////////////////////////

#define STORAGE_ADDR 0x08003700
void Flash_Unlock(){
    // Unkock flash - be aware you need extra stuff for the bootloader.
	FLASH->KEYR = FLASH_KEY1;
	FLASH->KEYR = FLASH_KEY2;

	// For option bytes.
//	FLASH->OBKEYR = FLASH_KEY1;
//	FLASH->OBKEYR = FLASH_KEY2;

	// For unlocking programming, in general.
	FLASH->MODEKEYR = FLASH_KEY1;
	FLASH->MODEKEYR = FLASH_KEY2;

    printf( "FLASH->CTLR = %08lx\n", FLASH->CTLR );
	if( FLASH->CTLR & 0x8080 ) 
	{
		printf( "Flash still locked\n" );
		while(1);
	}
}

void Flash_Erase(uint32_t* ptr){
	int start;
	int stop;

	//Erase Page
	FLASH->CTLR = CR_PAGE_ER;
	FLASH->ADDR = (intptr_t)ptr;
	FLASH->CTLR = CR_STRT_Set | CR_PAGE_ER;
	start = SysTick->CNT;
	while( FLASH->STATR & FLASH_STATR_BSY );  // Takes about 3ms.
	stop = SysTick->CNT;

	printf( "FLASH->STATR = %08lx -> %d cycles for page erase\n", FLASH->STATR, stop - start );
	printf( "Erase complete\n" );
}

void Flash_Write(uint32_t* ptr, uint32_t* copy_data, int len){
	int start;
	int stop;

	// Clear buffer and prep for flashing.
	FLASH->CTLR = CR_PAGE_PG;  // synonym of FTPG.
	FLASH->CTLR = CR_BUF_RST | CR_PAGE_PG;
	FLASH->ADDR = (intptr_t)ptr;  // This can actually happen about anywhere toward the end here.

	// Note: It takes about 6 clock cycles for this to finish.
	start = SysTick->CNT;
	while( FLASH->STATR & FLASH_STATR_BSY );  // No real need for this.
	stop = SysTick->CNT;
	printf( "FLASH->STATR = %08lx -> %d cycles for buffer reset\n", FLASH->STATR, stop - start );

    int i;
	start = SysTick->CNT;
	for( i = 0; i < len; i++ )
	{
		ptr[i] = 0xabcd1234 + i; //Write to the memory
		FLASH->CTLR = CR_PAGE_PG | FLASH_CTLR_BUF_LOAD; // Load the buffer.
		while( FLASH->STATR & FLASH_STATR_BSY );  // Only needed if running from RAM.
	}
	stop = SysTick->CNT;
	printf( "Write: %d cycles for writing data in\n", stop - start );

	// Actually write the flash out. (Takes about 3ms)
	FLASH->CTLR = CR_PAGE_PG|CR_STRT_Set;

	start = SysTick->CNT;
	while( FLASH->STATR & FLASH_STATR_BSY );
	stop = SysTick->CNT;
	printf( "FLASH->STATR = %08lx -> %d cycles for page write\n", FLASH->STATR, stop - start );
}

/// State Helpers ////////////////////////////////////////////////////////////
#define STATE_NAME_SIZE 24
#define STATE_STRUCT_SIZE 2
typedef struct BadgeState{ // 256 bits = 64 bit * 2
  uint64_t unlock_status; // 64 bits
} badge_state;

typedef union FlashPage1{
  uint32_t memory[STATE_STRUCT_SIZE];   // [128]; // 64 bits = 8 bytes
  badge_state state;
} flash_page_1;



int main()
{
    int start, stop;
	int testok = 1;

	SystemInit();

	Delay_Ms(100);

	printf( "Starting\n" );

	Flash_Unlock();

	uint32_t* ptr = (uint32_t*)STORAGE_ADDR;
	printf( "Memory at: %08lx: %08lx %08lx\n", (uint32_t)ptr, ptr[0], ptr[1] );
	printf( "FLASH->CTLR = %08lx\n", FLASH->CTLR );

    Flash_Erase(ptr);

	printf( "Memory at %p: %08lx %08lx\n", ptr, ptr[0], ptr[1] );
	if( ptr[0] != 0xffffffff )
	{
		printf( "WARNING/FAILURE: Flash general erasure failed\n" );
		testok = 0;
	}


    Flash_Write(ptr, 0, 16);

	

	printf( "FLASH->STATR = %08lx\n", FLASH->STATR );
	printf( "Memory at: %08lx: %08lx %08lx\n", (uint32_t)ptr, ptr[0], ptr[1] );


	if( ptr[0] != 0xabcd1234 )
	{
		printf( "WARNING/FAILURE: Flash general erasure failed\n" );
		testok = 0;
	}

	for(int i = 0; i < 16; i++ )
		printf( "%08lx ", ptr[i] );
	printf( "\n\nTest results: %s\n", testok?"PASS":"FAIL" );
	while(1);
}
