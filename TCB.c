#include "TCB.h"
#include "Pool.hpp"
#include <stdio.h>

Pool<Tcb_t, MAXNUMTHREADS> ThreadPool;
Tcb_t* RunningThread = NULL;
Tcb_t* SleepingThread = NULL; // Sleeping thread root
TcbListC_t ThreadList;
TcbListC_t SleepingList;

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
    Tcb_t* thread = RunningThread;
    if(ThreadList.count > 1){
        (RunningThread->prev)->next = RunningThread->next;
        (RunningThread->next)->prev = RunningThread->prev;
        ThreadList.count--;
    }else if(ThreadList.count == 1){
        RunningThread = NULL; 
        ThreadList.count--;
    } 
    ThreadPool.free(thread);
}

void TCB_RemoveRunningAndSleep(void) {
    Tcb_t* thread = RunningThread;
    if(ThreadList.count > 1){
        (RunningThread->prev)->next = RunningThread->next;
        (RunningThread->next)->prev = RunningThread->prev;
        ThreadList.count--;
    }else if(ThreadList.count == 1){
        RunningThread = NULL; 
        ThreadList.count--;
    } 
    TCB_AddSleeping(thread);
}
Tcb_t* TCB_GetRunningThread(void){
  return RunningThread;
}

int TCB_threadListEmpty(void){
    return (ThreadList.count == 0);
}


void TCB_AddSleeping(Tcb_t* node)
{
  if(SleepingList.count > 0){
    node->next = SleepingThread;
    node->prev = SleepingThread->prev;
    node->prev->next = node;
    SleepingThread->prev = node;
  }
  else {
    SleepingList.head = node;
    SleepingThread = node;
  }
  SleepingList.count++;
}

void TCB_RemoveSleepingNode(Tcb_t* thread){
  if(SleepingList.count > 1){
        (thread->prev)->next = thread->next;
        (thread->next)->prev = thread->prev;
        SleepingList.count--;
    }else if(SleepingList.count == 1){
        SleepingThread = NULL; 
        SleepingList.count--;
    } 
    thread->next = thread;
    thread->prev = thread;
}
// void TCB_RemoveSleepingAndAdd2Run(void) {
//     Tcb_t* sleepingNode = SleepingList.head;
    
//     while(sleepingNode->next != SleepingList.head){
//     if(sleepingNode->state_sleep < 1){
//       TCB_RemoveSleepingNode(sleepingNode);
//       TCB_InsertNodeBeforeRoot(sleepingNode);
//     }
//     sleepingNode = sleepingNode->next;
//   }
// }

void TCB_UpdateSleeping(void) {
    Tcb_t* sleepingNode = SleepingList.head;
    
    while(sleepingNode->next != SleepingList.head){
      sleepingNode->state_sleep--;

      if(sleepingNode->state_sleep < 1){
        TCB_RemoveSleepingNode(sleepingNode);
        TCB_InsertNodeBeforeRoot(sleepingNode);
    }
    sleepingNode = sleepingNode->next;
  }
}
/*
void dummy(void){
  printf("Attempting Task\n");
  void (*Task)(void);
  Task = (void (*)(void)) RunningThread->stack[STACKSIZE - 2];
  Task();
}
*/
