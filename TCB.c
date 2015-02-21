#include "TCB.h"
#include "Pool.hpp"
#include <stdio.h>
#include "sleepList.hpp"
// #include <iostream>
#define DEBUGprintf(...) /**/

List<Tcb_t*, MAXNUMTHREADS> SleepingList2;
Pool<Tcb_t, MAXNUMTHREADS> ThreadPool;
Tcb_t* RunningThread = NULL;
Tcb_t* NextRunningThread = NULL;
Tcb_t* SleepingThread = NULL; // Sleeping thread root
TcbListC_t ThreadList;
TcbListC_t SleepingList;
Tcb_t DummyThread;

//sleepListS_t SleepingList;
Tcb_t* idleThread = NULL;

void Idle(void){
  uint32_t idleTime;
  while(1){
    //Idle
    idleTime++;
  }
}

void (*idleTask)(void);
/**
 * @brief Configure the idle thread
 * @details The idle thread exists when the scheduler is empty.
 * This way the code does not hardfault on an invalid context switch. In
 * other words, there are no threads to switch to in the list and we do 
 * not want to context switch to null.
 */
void TCB_Configure_IdleThread(void){
  idleTask = Idle;
  idleThread = TCB_GetNewThread();
  TCB_SetInitialStack(idleThread);
  // idleThread->stack[STACKSIZE-2] = (int32_t) (Idle); //return to IDLE
  // idleThread->stack[STACKSIZE-2] = Idle; //return to IDLE
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
    node->next = (RunningThread->next)->prev; //!< NOTE: This should fix a lot of problems
    node->prev = RunningThread->prev;
    node->prev->next = node;
    RunningThread->prev = node;
  }
  else {
    ThreadList.head = node;  //<! Replace Idle Thread with node
    // RunningThread->next = node;    //<! Replace Idle Thread with node
    // RunningThread->prev = node;    //<! Replace Idle Thread with node
    RunningThread = node;
  }
  ThreadList.count++;
}

void TCB_RemoveThread(Tcb_t* thread){
  if(ThreadList.count > 1){
        (thread->prev)->next = thread->next;
        (thread->next)->prev = thread->prev;
        if(thread == ThreadList.head){
          ThreadList.head = ThreadList.head->next;
        }
        // if(thread == RunningThread){
        //   RunningThread = RunningThread->next;
        // }
        //thread = thread->prev;  //Roll back running thread so that we point to the right
                                              // Location after context switching
        ThreadList.count--;
    }else if(ThreadList.count == 1){
        // thread = NULL;
        RunningThread->next = idleThread;
        idleThread->next = idleThread;
        idleThread->prev = idleThread;

        thread->next = idleThread;   //Make Sure to never call sleep on idle thread
        ThreadList.head = idleThread; //Make Sure to never call sleep on idle thread 
        ThreadList.count--;
    } 
    ThreadPool.free(thread);
}
void TCB_RemoveThreadAndSleep(Tcb_t* thread) {
    // Tcb_t* thread = RunningThread;
    if(ThreadList.count > 1){
        (thread->prev)->next = thread->next;
        (thread->next)->prev = thread->prev;
        if(thread == ThreadList.head){
          ThreadList.head = ThreadList.head->next;
        }
        // if(thread == RunningThread){
        //   RunningThread = RunningThread->next;
        // }

        //thread = thread->prev;  //Roll back running thread so that we point to the right
                                              // Location after context switching
        // DummyThread.sp = thread->sp;
        // DummyThread.next = thread->next;
        // thread = &DummyThread;
       
        ThreadList.count--;
    }else if(ThreadList.count == 1){
        // thread = NULL; 
        RunningThread->next = idleThread;

        idleThread->next = idleThread;
        thread->next = idleThread;   //Make Sure to never call sleep on idle thread
        idleThread->prev = idleThread;

        ThreadList.head = idleThread; //Make Sure to never call sleep on idle thread 

        // thread = &DummyThread;
        ThreadList.count--;
    } 
    // TCB_AddSleeping(thread);
    SleepingList2.push_back(thread);
    //SleepingList2.Add(thread);
    thread = &DummyThread;

}
void TCB_RemoveRunningThread(void) {
    Tcb_t* thread = RunningThread;
    if(ThreadList.count > 1){
        (RunningThread->prev)->next = RunningThread->next;
        (RunningThread->next)->prev = RunningThread->prev;
				//RunningThread = RunningThread->prev;	//Roll back running thread so that we point to the right
																							// Location after context switching
        // DummyThread.sp = RunningThread->sp;
        // DummyThread.next = RunningThread->next;
        // RunningThread = &DummyThread;

        ThreadList.count--;
    }else if(ThreadList.count == 1){
        // RunningThread = NULL;
      idleThread->next = idleThread;
        RunningThread->next = idleThread;   //Make Sure to never call sleep on idle thread
        ThreadList.head = idleThread; //Make Sure to never call sleep on idle thread 
        ThreadList.count--;
    } 
    ThreadPool.free(thread);
}

void TCB_RemoveRunningAndSleep(void) {
    // Tcb_t* thread = RunningThread;
    // DummyThread.sp = RunningThread->sp;
    // DummyThread.next = RunningThread->next;
    if(ThreadList.count > 1){
        (RunningThread->prev)->next = RunningThread->next;
        (RunningThread->next)->prev = RunningThread->prev;
				//RunningThread = RunningThread->prev;	//Roll back running thread so that we point to the right
																							// Location after context switching
        // DummyThread.sp = RunningThread->sp;
        // DummyThread.next = RunningThread->next;
        // RunningThread = &DummyThread;
       
        ThreadList.count--;
    }else if(ThreadList.count == 1){
        // RunningThread = NULL; 
        RunningThread->next = idleThread;   //Make Sure to never call sleep on idle thread
        ThreadList.head = idleThread; //Make Sure to never call sleep on idle thread 

        // RunningThread = &DummyThread;
        ThreadList.count--;
    } 
    DEBUGprintf("Sleeping\n");
    // TCB_AddSleeping(RunningThread);
    SleepingList2.push_back(RunningThread);
    // SleepingList2.Add(RunningThread);
    // RunningThread = &DummyThread;

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
/*
void TCB_UpdateSleeping(void) {
  SleepingList2.UpdateSleeping();
  Tcb_t* head;
  Tcb_t* tail;
  int res;
  res = SleepingList2.GetReadyList(head, tail);
  if(res > 0){


  (RunningThread->prev)->next = head;
  head->prev = RunningThread->prev;
  RunningThread->prev = tail;
  tail->next = RunningThread;
  }
}
*/
void TCB_UpdateSleeping(void) {
  //   Tcb_t* sleepingNode = SleepingList.head;
  //   if(sleepingNode == NULL){
  //     return;
  //   }
  //   //while(sleepingNode->next != SleepingList.head){
  //   while(sleepingNode->next != NULL){
		// 	// if(SleepingList.count == 0){
		// 	// 	break;
		// 	// 	}
  //     sleepingNode->state_sleep--;

  //     if(sleepingNode->state_sleep < 1){
  //       Tcb_t* prev = sleepingNode->prev;
  //       TCB_RemoveSleepingNode(sleepingNode);
  //       TCB_InsertNodeBeforeRoot(sleepingNode);
  //       sleepingNode = prev;
  //   }
  //   sleepingNode = sleepingNode->next;
  // }
  List<Tcb_t*, MAXNUMTHREADS>::iterator iter;
  Tcb_t* thread;
  for(iter = SleepingList2.begin(); iter != SleepingList2.end(); ++iter){
    ((*iter)->state_sleep)--;
    if((*iter)->state_sleep < 1){
      // thread = SleepingList2.Get(iter.getCell());
      DEBUGprintf("Found Ready Thread\n");
      thread = *iter;
      thread->next = thread;
      thread->prev = thread;
      thread->state_sleep = 0;
      // ++iter;
      // SleepingList2.remove(thread);
      
      // iter = SleepingList2.erase(iter);
      iter.mark4Delete();
      TCB_InsertNodeBeforeRoot(thread);
      

    }
  }
  SleepingList2.clean();
}

void TCB_CheckSleeping(void) {
    Tcb_t* sleepingNode = SleepingList.head;
    if(sleepingNode == NULL){
      return;
    }
    while(sleepingNode->next != SleepingList.head){
      // while(sleepingNode->next != NULL)
      // if(SleepingList.count == 0){
      //   break;
      //   }
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
void ContextSwitch(void){
  RunningThread = RunningThread->next;
}
