#include "TCB.h"
#include "Pool.hpp"

Pool<Tcb_t, MAXNUMTHREADS> ThreadPool;
Tcb_t* RunningThread; 
TcbListC_t ThreadList;

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
  Tcb_t* thread =  ThreadPool.get();
  thread->next = thread;
  thread->prev = thread;
  thread->id = ThreadPool.getId();
  return thread;
}

void TCB_InsertNodeBeforeRoot(Tcb_t* node)
{
  if(ThreadList.count > 0){
    node->next = RunningThread;
    node->prev = RunningThread->prev;
    node->prev->next = node;
    RunningThread->prev = node;
  }
  else {
    ThreadList.head = node;
    RunningThread = node;
  }
}
