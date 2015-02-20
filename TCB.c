#include "TCB.h"
#include "Pool.hpp"
#include <stdio.h>

Pool<Tcb_t, MAXNUMTHREADS> ThreadPool;
Tcb_t* RunningThread = NULL;
Tcb_t* SleepingThread = NULL; // Sleeping thread root
TcbListC_t ThreadList;
TcbListC_t SleepingList;
//sleepListS_t SleepingList;
Tcb_t* idleThread = NULL;

void Idle(void){
  uint32_t idleTime;
  while(1){
    //Idle
    idleTime++;
  }
}

/**
 * @brief Configure the idle thread
 * @details The idle thread exists when the scheduler is empty.
 * This way the code does not hardfault on an invalid context switch. In
 * other words, there are no threads to switch to in the list and we do 
 * not want to context switch to null.
 */
void TCB_Configure_IdleThread(void){
  idleThread = TCB_GetNewThread();
  TCB_SetInitialStack(idleThread);
  idleThread->stack[STACKSIZE-2] = (int32_t) (Idle); //return to IDLE
  ThreadList.head = idleThread;
  RunningThread = idleThread;

}

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
    ThreadList.head = node;  //<! Replace Idle Thread with node
    RunningThread = node;    //<! Replace Idle Thread with node
  }
  ThreadList.count++;
}


void TCB_RemoveRunningThread(void) {
    Tcb_t* thread = RunningThread;
    if(ThreadList.count > 1){
        (RunningThread->prev)->next = RunningThread->next;
        (RunningThread->next)->prev = RunningThread->prev;
				RunningThread = RunningThread->prev;	//Roll back running thread so that we point to the right
																							// Location after context switching

        ThreadList.count--;
    }else if(ThreadList.count == 1){
        // RunningThread = NULL;
        RunningThread = idleThread;   //Make Sure to never call sleep on idle thread
        ThreadList.head = idleThread; //Make Sure to never call sleep on idle thread 
        ThreadList.count--;
    } 
    ThreadPool.free(thread);
}

void TCB_RemoveRunningAndSleep(void) {
    Tcb_t* thread = RunningThread;
    if(ThreadList.count > 1){
        (RunningThread->prev)->next = RunningThread->next;
        (RunningThread->next)->prev = RunningThread->prev;
				RunningThread = RunningThread->prev;	//Roll back running thread so that we point to the right
																							// Location after context switching
        ThreadList.count--;
    }else if(ThreadList.count == 1){
        // RunningThread = NULL; 
        RunningThread = idleThread;   //Make Sure to never call sleep on idle thread
        ThreadList.head = idleThread; //Make Sure to never call sleep on idle thread 

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
  // sleepElem_t* elem = SleepingList.head;
  Tcb_t* elem = SleepingList.head;
  if(SleepingList.count > 0){
    // node->next = SleepingThread;
    // node->prev = SleepingThread->prev;
    // node->prev->next = node;
    // SleepingThread->prev = node;
    while(elem->next != NULL){
      elem = elem->next;
    }
    elem->next = node;
    node->next = NULL;
    node->prev = elem;
  }
  else {
    SleepingList.head = node;
    // SleepingThread = node;
    node->next = NULL;
    node->prev = NULL;
  }
  SleepingList.count++;
}

void TCB_RemoveSleepingNode(Tcb_t* thread){
  if(SleepingList.count > 1){
      if(thread == SleepingList.head){
        SleepingList.head = SleepingList.head->next;
      }
      (thread->prev)->next = thread->next;
      (thread->next)->prev = thread->prev;
      SleepingList.count--;
    }else if(SleepingList.count == 1){
			  // SleepingThread->next = NULL;
				// SleepingThread->prev = NULL;
        // SleepingThread = NULL; 
        // SleepingList.head->next = NULL;
        // SleepingList.head->prev = NULL;
				SleepingList.head = NULL;
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
    if(sleepingNode == NULL){
      return;
    }
    while(sleepingNode->next != SleepingList.head){
      // while(sleepingNode->next != NULL)
			if(SleepingList.count == 0){
				break;
				}
      sleepingNode->state_sleep--;

      if(sleepingNode->state_sleep < 1){
        Tcb_t* prev = sleepingNode->prev;
        TCB_RemoveSleepingNode(sleepingNode);
        TCB_InsertNodeBeforeRoot(sleepingNode);
        sleepingNode = prev;
    }
    sleepingNode = sleepingNode->next;
  }
}
void TCB_CheckSleeping(void) {
    Tcb_t* sleepingNode = SleepingList.head;
    if(sleepingNode == NULL){
      return;
    }
    while(sleepingNode->next != SleepingList.head){
      // while(sleepingNode->next != NULL)
      if(SleepingList.count == 0){
        break;
        }
      // sleepingNode->state_sleep--;

      if(sleepingNode->state_sleep < 1){
        Tcb_t* prev = sleepingNode->prev;
        TCB_RemoveSleepingNode(sleepingNode);
        TCB_InsertNodeBeforeRoot(sleepingNode);
        sleepingNode = prev;
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
