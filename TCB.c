#include "TCB.h"
#include "Pool.hpp"

Pool<Tcb_t, MAXNUMTHREADS> ThreadPool;
Tcb_t* RunningThread; 

void TCB_SetInitialStack(Tcb_t* pTcb)
{
  int32_t* stack = pTcb->stack;
  pTcb->sp = &stack[STACKSIZE - 16];
  stack[STACKSIZE-1] = 0x01000000; // Set Thumb bit
  //Rest of stack registers are currently random
}

int TCB_Available(void){
  return ThreadPool.available();
}

Tcb_t* TCB_GetNewThread(void){
  return ThreadPool.get();
}
