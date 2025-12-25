#include "ch32fun.h"
#include <stdio.h>
/// Flash Helpers ////////////////////////////////////////////////////////////

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

void Flash_Lock(){
    FLASH->CTLR = FLASH_CTLR_LOCK;
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
    if( ptr[0] != 0xffffffff ) {
		printf( "WARNING/FAILURE: Flash general erasure failed\n" );
	}
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
		// ptr[i] = 0xabcd1234 + i; //Write to the memory
        ptr[i] = copy_data[i]; //Write to the memory
		printf( "%08lx(%d) ", copy_data[i], i);
		FLASH->CTLR = CR_PAGE_PG | FLASH_CTLR_BUF_LOAD; // Load the buffer.
		while( FLASH->STATR & FLASH_STATR_BSY );  // Only needed if running from RAM.
	}
	stop = SysTick->CNT;
	printf( "\nWrite: %d cycles for writing data in\n", stop - start );

	// Actually write the flash out. (Takes about 3ms)
	FLASH->CTLR = CR_PAGE_PG|CR_STRT_Set;

	start = SysTick->CNT;
	while( FLASH->STATR & FLASH_STATR_BSY );
	stop = SysTick->CNT;
	printf( "FLASH->STATR = %08lx -> %d cycles for page write\n", FLASH->STATR, stop - start );
}

/// State Helpers ////////////////////////////////////////////////////////////
#define STORAGE_ADDR 0x08003f00
#define STATE_STRUCT_SIZE 2

typedef struct BadgeState{ 
  uint64_t unlock_status; // 64 bits
} badge_state;

typedef union FlashPage1{
  uint32_t memory[STATE_STRUCT_SIZE];   // [128]; // 64 bits = 8 bytes / 4 = 2 bytes
  badge_state state;
} flash_page_1;

badge_state BadgeState_Read(){
    badge_state *badge_state_addr = (badge_state *) STORAGE_ADDR;
    return *badge_state_addr;
}

void BadgeState_Erase(){
    Flash_Unlock();
    Flash_Erase((uint32_t*)STORAGE_ADDR);
    Flash_Lock();
}

void BadgeState_Write(badge_state st){
    flash_page_1 page;
    page.state = st;

    Flash_Unlock();
    Flash_Erase((uint32_t*)STORAGE_ADDR); // NEED TO ERASE - set all to 0xffffffff
    Flash_Write((uint32_t*)STORAGE_ADDR, (uint32_t*)(page.memory), STATE_STRUCT_SIZE); // like an & operation, only keep bits
    Flash_Lock();
}


int BadgeState_Unlocked(badge_state st, int index){
	return (st.unlock_status & (1<<index)) > 0;
}

void BadgeState_SetUnlocked(badge_state st, int index){
	st.unlock_status |= (1<<index);
}