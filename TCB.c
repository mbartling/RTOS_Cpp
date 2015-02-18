#include "TCB.h"
#include "Pool.hpp"
#include <stdio.h>

Pool<Tcb_t, MAXNUMTHREADS> ThreadPool;
Tcb_t* RunningThread = NULL;
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
  ThreadList.count++;
}


void TCB_RemoveRunningThread(void) {
    if(ThreadList.count > 1){
        (RunningThread->prev)->next = RunningThread->next;
        (RunningThread->next)->prev = RunningThread->prev;
        ThreadList.count--;
    }else if(ThreadList.count == 1){
        RunningThread = NULL; 
        ThreadList.count--;
    } 
}

Tcb_t* TCB_GetRunningThread(void){
  return RunningThread;
}

int TCB_threadListEmpty(void){
    return (ThreadList.count == 0);
}



/*
void dummy(void){
  printf("Attempting Task\n");
  void (*Task)(void);
  Task = (void (*)(void)) RunningThread->stack[STACKSIZE - 2];
  Task();
}
*/
