#include "Pool.hpp"
//#include "UART0.h"
//#include "PLL.h"
#include <stdio.h>
#include "Stack.hpp"
#include "os.h"
#include "TCB.h"
//Pool<int, 50> mPool;
//Stack<512> mStack;
//#include "ST7735.h"
//#include "ADC.h"
//#include "UART2.h"
#include <string.h> 
//*********Prototype for FFT in cr4_fft_64_stm32.s, STMicroelectronics
unsigned long Count1;   // number of times thread1 loops
unsigned long Count2;   // number of times thread2 loops
unsigned long Count3;   // number of times thread3 loops
unsigned long Count4;   // number of times thread4 loops
unsigned long Count5;   // number of times thread5 loops
void Thread1(void){
  Count1 = 0;          
  for(;;){
    PE0 ^= 0x01;       // heartbeat
    Count1++;
    OS_Suspend();      // cooperative multitasking
  }
}
void Thread2(void){
  Count2 = 0;          
  for(;;){
    PE1 ^= 0x02;       // heartbeat
    Count2++;
    OS_Suspend();      // cooperative multitasking
  }
}
void Thread3(void){
  Count3 = 0;          
  for(;;){
    PE2 ^= 0x04;       // heartbeat
    Count3++;
    OS_Suspend();      // cooperative multitasking
  }
}

int main(void){  // Testmain1
  OS_Init();          // initialize, disable interrupts
  PortE_Init();       // profile user threads
  NumCreated = 0 ;
  NumCreated += OS_AddThread(&Thread1,128,1); 
  NumCreated += OS_AddThread(&Thread2,128,2); 
  NumCreated += OS_AddThread(&Thread3,128,3); 
  // Count1 Count2 Count3 should be equal or off by one at all times
  OS_Launch(TIME_2MS); // doesn't return, interrupts enabled in here
  return 0;            // this never executes
}
//*******************Second TEST**********
// Once the initalize test runs, test this (Lab 1 part 1)
// no UART interrupts
// SYSTICK interrupts, with or without period established by OS_Launch
// no timer interrupts
// no switch interrupts
// no ADC serial port or LCD output
// no calls to semaphores
void Thread1b(void){
  Count1 = 0;          
  for(;;){
    PE0 ^= 0x01;       // heartbeat
    Count1++;
  }
}
void Thread2b(void){
  Count2 = 0;          
  for(;;){
    PE1 ^= 0x02;       // heartbeat
    Count2++;
  }
}
void Thread3b(void){
  Count3 = 0;          
  for(;;){
    PE2 ^= 0x04;       // heartbeat
    Count3++;
  }
}
int Testmain2(void){  // Testmain2
  OS_Init();           // initialize, disable interrupts
  PortE_Init();       // profile user threads
  NumCreated = 0 ;
  NumCreated += OS_AddThread(&Thread1b,128,1); 
  NumCreated += OS_AddThread(&Thread2b,128,2); 
  NumCreated += OS_AddThread(&Thread3b,128,3); 
  // Count1 Count2 Count3 should be equal on average
  // counts are larger than testmain1
 
  OS_Launch(TIME_2MS); // doesn't return, interrupts enabled in here
  return 0;            // this never executes
}

//******************* Lab 3 Measurement of context switch time**********
// Run this to measure the time it takes to perform a task switch
// UART0 not needed 
// SYSTICK interrupts, period established by OS_Launch
// first timer not needed
// second timer not needed
// SW1 not needed, 
// SW2 not needed
// logic analyzer on PF1 for systick interrupt (in your OS)
//                on PE0 to measure context switch time
void Thread8(void){       // only thread running
  while(1){
    PE0 ^= 0x01;      // debugging profile  
  }
}
int Testmain7(void){       // Testmain7
  PortE_Init();
  OS_Init();           // initialize, disable interrupts
  NumCreated = 0 ;
  NumCreated += OS_AddThread(&Thread8,128,2); 
  OS_Launch(TIME_1MS/10); // 100us, doesn't return, interrupts enabled in here
  return 0;             // this never executes
}

