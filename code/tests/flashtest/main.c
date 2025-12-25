#include "ch32fun.h"
#include <stdio.h>
#include "badge_state.h"


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

    Flash_Erase(ptr);
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

    BadgeState_Erase();
    st = BadgeState_Read();
    printf("Status: %x\n", st.unlock_status);

    // Badge State Example
    st.unlock_status = 0x12345678deadbeef;
    // st.unlock_status = 0xffffffffffff0000;
    BadgeState_Write(st);


    printf( "\n\n" );
	for( int i = 0; i < 16; i++ )
		printf( "%08lx ", ptr[i] );
	printf( "\n\n" );


    st = BadgeState_Read();
    printf("Status: %x\n", st.unlock_status);

	while(1);
}
