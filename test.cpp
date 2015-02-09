#include "Pool.hpp"
//#include "UART0.h"
//#include "PLL.h"
#include <stdio.h>
#include "Stack.hpp"
#include "os.h"
#include "TCB.h"
//Pool<int, 50> mPool;
//Stack<512> mStack;

void dummy1(void){
  printf("Hi there\n");
}

int main(){
 //  PLL_Init();
 //  UART0_Init();
	// printf("Testing the Main\n");
 //  printf("Pool Available: %d\n", mPool.available());

	// printf("Amount of stack space left: %d\n", mStack.available());

 //  void* pv1 = mStack.get(64);
 //  int* buffer1 = static_cast<int*>(pv1);
	// printf("Amount of stack space left: %d\n", mStack.available());
	// void* pv2 = mStack.get(64);
 //  int* buffer2 = static_cast<int*>(pv1);
	// printf("Amount of stack space left: %d\n", mStack.available());
	
	// mStack.free();
	// printf("Amount of stack space left: %d\n", mStack.available());
	// mStack.free();
	// printf("Amount of stack space left: %d\n", mStack.available());
	
  OS_Init();

  OS_AddThread(dummy1, 255, 5);
  //dummy();

  OS_Launch(80000);

  printf("Done\n");
  return 0;  
}
