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

// Should be a lock
void Flash_Lock(){
    FLASH->KEYR = 0;
	FLASH->KEYR = 0;

	FLASH->MODEKEYR = 0;
	FLASH->MODEKEYR = 0;

	// if( FLASH->CTLR & 0x8080 )
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
#define STORAGE_ADDR 0x08003700
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
    Flash_Erase((uint32_t*)STORAGE_ADDR);
}

void BadgeState_Write(badge_state st){
    flash_page_1 page;
    page.state = st;

    Flash_Unlock();
    Flash_Write((uint32_t*)STORAGE_ADDR, (uint32_t*)(page.memory), STATE_STRUCT_SIZE);
    // Flash_Lock();
}


int main1(){
	SystemInit();
	Delay_Ms(100);
    uint32_t* ptr = (uint32_t*)STORAGE_ADDR;


    badge_state st;
    st = BadgeState_Read();
    printf("Status: %x\n", st.unlock_status);

    BadgeState_Erase();
    st = BadgeState_Read();
    printf("Status: %x\n", st.unlock_status);

    // Badge State Example
    st.unlock_status = 0xdeadbeefdeadbeef;
    BadgeState_Write(st);


    printf( "\n\n" );
	for( int i = 0; i < 16; i++ )
		printf( "%08lx ", ptr[i] );
	printf( "\n\n" );


    st = BadgeState_Read();
    printf("Status: %x\n", st.unlock_status);

	while(1);
}

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
	if( ptr[0] != 0xffffffff ) {
		printf( "WARNING/FAILURE: Flash general erasure failed\n" );
		testok = 0;
	}


	uint32_t copydata[16];
	for(int i = 0; i < 16; i++ ) { copydata[i] = 0xabcd1234 + i+i; }
    Flash_Write(ptr, copydata, 16);

	printf( "FLASH->STATR = %08lx\n", FLASH->STATR );
	printf( "Memory at: %08lx: %08lx %08lx\n", (uint32_t)ptr, ptr[0], ptr[1] );


	if( ptr[0] != 0xabcd1234 ) {
		printf( "WARNING/FAILURE: Flash general erasure failed\n" );
		testok = 0;
	}

	for( int i = 0; i < 16; i++ )
		printf( "%08lx ", ptr[i] );
	printf( "\n\nTest results: %s\n", testok?"PASS":"FAIL" );


    ////////////////////////////////////////////////////////////////////////////////
    // Locking doesn't really work tbh
    Flash_Lock();

	for(int i = 0; i < 16; i++ ) { copydata[i] = 0xabcd1237 + i+i; }
    Flash_Write(ptr, copydata, 16);

    printf( "\n\n" );
	for( int i = 0; i < 16; i++ )
		printf( "%08lx ", ptr[i] );
	printf( "\n\n" );

        
    ////////////////////////////////////////////////////////////////////////////////
    badge_state st;
    st = BadgeState_Read();
    printf("Status: %x\n", st.unlock_status);

    // Badge State Example
    st.unlock_status = 0x12345678deadbeef;
    st.unlock_status = 0xffffffffffff0000;
    BadgeState_Write(st);


    printf( "\n\n" );
	for( int i = 0; i < 16; i++ )
		printf( "%08lx ", ptr[i] );
	printf( "\n\n" );


    st = BadgeState_Read();
    printf("Status: %x\n", st.unlock_status);

	while(1);
}
