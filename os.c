#include "os.h"
#include "TCB.h"
#include "PLL.h"
#include "UART0.h"
#include <stdint.h>
#include <stdio.h>
#include "SysTickInts.h"
#include "inc/tm4c123gh6pm.h"
#include "Switch.h"
#include "Timer.h"
#define STACKSIZE 100
#define SYSTICK_EN 1  
#ifdef __cplusplus
extern "C" {
#endif
//comment the following line to deactivate Systick
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode
void StartOS(void);
void ContextSwitch(void);
void GPIOPortF_Handler(void);
// void PendSV_Handler(void);
#ifdef __cplusplus
}
#endif
unsigned long Time = 0;
int32_t ThreadCount = 0;
// Tcb_t idleThreadMem;
// Tcb_t* idleThread = &idleThreadMem;

inline void Context_Switch(void){
    NVIC_INT_CTRL_R = NVIC_INT_CTRL_PEND_SV;
}
 /********* OS_Init ************
 * initialize operating system, disable interrupts until OS_Launch
 * initialize OS controlled I/O: serial, ADC, systick, LaunchPad I/O and timers 
 * @param  input:  none
 * @return output: none
 */
void OS_Init(void)
{
  DisableInterrupts();
  PLL_Init();
  NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R& ~NVIC_SYS_PRI3_PENDSV_M)|0x00E00000; // priority 6

#ifdef SYSTICK_EN
  SysTick_Init(160000); //2 Ms period default
#endif
  UART0_Init();
  TCB_Configure_IdleThread(); //Set up the idle thread
}



// ******** OS_Wait ************
// decrement semaphore 
// Lab2 spinlock
// Lab3 block if less than zero
// input:  pointer to a counting semaphore
// output: none
// the following defintion is suitable for coopeartive semaphores
void OS_Wait(Sema4Type *semaPt) {
    DisableInterrupts();
    //while(__ldrex(&(semaPt->Value)) <= 0){
		while(semaPt->Value <= 0){
        EnableInterrupts();
        OS_Suspend();
        DisableInterrupts();
    }
    (semaPt->Value) = (semaPt->Value) - 1;
		//while(!__strex(__ldrex(&(semaPt->Value)) - 1, &(semaPt->Value))){}
    EnableInterrupts();
}

// ******** OS_Signal ************
// increment semaphore 
// Lab2 spinlock
// Lab3 wakeup blocked thread if appropriate 
// input:  pointer to a counting semaphore
// output: none
void OS_Signal(Sema4Type *semaPt) {
    long status;
    status = StartCritical();
    (semaPt->Value) = (semaPt->Value) + 1;
    EndCritical(status);
}

// ******** OS_bWait ************
// Lab2 spinlock, set to 0
// Lab3 block if less than zero
// input:  pointer to a binary semaphore
// output: none
void OS_bWait(Sema4Type *semaPt) {
    OS_Wait(semaPt);
}

// ******** OS_bSignal ************
// Lab2 spinlock, set to 1
// Lab3 wakeup blocked thread if appropriate 
// input:  pointer to a binary semaphore
// output: none
void OS_bSignal(Sema4Type *semaPt) {
    long status;
    status = StartCritical();
    if ((semaPt->Value) == 0){
       semaPt->Value =  (semaPt->Value) + 1;
    } 
    EndCritical(status);
}


// ******** OS_InitSemaphore ************
// initialize semaphore 
// input:  pointer to a semaphore
// output: none
void OS_InitSemaphore(Sema4Type *semaPt, long value) {
    semaPt->Value = value;
}

//******** OS_AddThread *************** 
// add a foregound thread to the scheduler
// Inputs: pointer to a void/void foreground task
//         number of bytes allocated for its stack
//         priority, 0 is highest, 5 is the lowest
// Outputs: 1 if successful, 0 if this thread can not be added
// stack size must be divisable by 8 (aligned to double word boundary)
// In Lab 2, you can ignore both the stackSize and priority fields
// In Lab 3, you can ignore the stackSize fields
int OS_AddThread(void(*task)(void), 
  unsigned long stackSize, unsigned long priority){
  if(priority > 5){
    return 0;
  }
  long status;
  status = StartCritical();
  // if(ThreadCount > (MAXNUMTHREADS - 1)){
  if(!TCB_Available())
	{
    EndCritical(status);
    return 0;
  }

  // Tcb_t* thread = &TcbTable[ThreadCount];
  Tcb_t* thread = TCB_GetNewThread();
  TCB_SetInitialStack(thread);  //Set thumb bit and dummy regs
  thread->stack[STACKSIZE-2] = (int32_t) (task); //return to task
  thread->priority = priority;
  TCB_InsertNodeBeforeRoot(thread);

  ThreadCount++;
  EndCritical(status);
  return 1;
}

// int AddIdleThread(void){

//   long status;
//   status = StartCritical();
//   // if(ThreadCount > (MAXNUMTHREADS - 1)){
//   if(!TCB_Available())
//   {
//     EndCritical(status);
//     return 0;
//   }

//   // Tcb_t* thread = &TcbTable[ThreadCount];
//   Tcb_t* thread = TCB_GetNewThread();
//   TCB_SetInitialStack(thread);  //Set thumb bit and dummy regs
//   thread->stack[STACKSIZE-2] = (int32_t) (task); //return to task
//   thread->priority = priority;
//   TCB_InsertNodeBeforeRoot(thread);

//   ThreadCount++;
//   EndCritical(status);
//   return 1;
// }


//******** OS_Id *************** 
// returns the thread ID for the currently running thread
// Inputs: none
// Outputs: Thread ID, number greater than zero 
unsigned long OS_Id(void) {
	Tcb_t * runningThread = TCB_GetRunningThread();
	return runningThread->id;
}

//******** OS_AddPeriodicThread *************** 
// add a background periodic task
// typically this function receives the highest priority
// Inputs: pointer to a void/void background function
//         period given in system time units (12.5ns)
//         priority 0 is the highest, 5 is the lowest
// Outputs: 1 if successful, 0 if this thread can not be added
// You are free to select the time resolution for this function
// It is assumed that the user task will run to completion and return
// This task can not spin, block, loop, sleep, or kill
// This task can call OS_Signal  OS_bSignal	 OS_AddThread
// This task does not have a Thread ID
// In lab 2, this command will be called 0 or 1 times
// In lab 2, the priority field can be ignored
// In lab 3, this command will be called 0 1 or 2 times
// In lab 3, there will be up to four background threads, and this priority field 
//           determines the relative priority of these four threads

void (*GlobalPeriodicThread)(void);
unsigned long GlobalPeriodicThreadPriority;
int OS_AddPeriodicThread(void(*task)(void), 
        unsigned long period, unsigned long priority){
    long status = StartCritical();
    GlobalPeriodicThread = task;
    Timer_Init((uint32_t)period);
    EndCritical(status);
    return 1;
}

// ******** OS_Sleep ************
// place this thread into a dormant state
// input:  number of msec to sleep
// output: none
// You are free to select the time resolution for this function
// OS_Sleep(0) implements cooperative multitasking
void OS_Sleep(unsigned long sleepTime) {
    long status = StartCritical();
    Tcb_t * runningThread = TCB_GetRunningThread();
    runningThread-> state_sleep = sleepTime;
    TCB_RemoveRunningAndSleep();
	  EndCritical(status);
    Context_Switch();
//    NVIC_INT_CTRL_R = NVIC_INT_CTRL_PEND_SV;
 
}

// ******** OS_Kill ************
// kill the currently running thread, release its TCB and stack
// input:  none
// output: none
void OS_Kill(void) {
    long status = StartCritical();
    TCB_RemoveRunningThread();
		ThreadCount--;
    //if TCB_ThreadList is not empty after removing the current thread, context switch
    // if(TCB_threadListEmpty != 0) {
        // OS_Suspend(); 
    // }
	
	
    EndCritical(status); 
    Context_Switch();

    // NVIC_INT_CTRL_R = NVIC_INT_CTRL_PEND_SV;

}


// ******** OS_Suspend ************
// suspend execution of currently running thread
// scheduler will choose another thread to execute
// Can be used to implement cooperative multitasking 
// Same function as OS_Sleep(0)
// input:  none
// output: none
void OS_Suspend(void)
{
		
	  long status = StartCritical();
    Tcb_t * runningThread = TCB_GetRunningThread();
    runningThread-> state_sleep = 0;
    TCB_RemoveRunningAndSleep();
  //  TCB_UpdateSleeping();
    // TCB_CheckSleeping();
    //If running list empty then insert idle task
	  EndCritical(status);
  //This is where we would do any scheduling
	
    Context_Switch();
  // NVIC_INT_CTRL_R = NVIC_INT_CTRL_PEND_SV;
}

//******** OS_Launch *************** 
// start the scheduler, enable interrupts
// Inputs: number of 12.5ns clock cycles for each time slice
//         you may select the units of this parameter
// Outputs: none (does not return)
// In Lab 2, you can ignore the theTimeSlice field
// In Lab 3, you should implement the user-defined TimeSlice field
// It is ok to limit the range of theTimeSlice to match the 24-bit SysTick
void OS_Launch(unsigned long theTimeSlice)
{
#ifdef SYSTICK_EN
	
  NVIC_ST_RELOAD_R = theTimeSlice - 1;
//	NVIC_ST_CURRENT_R = 0;      // any write to current clears it
  NVIC_ST_CTRL_R = 0x00000007;  //Enable core clock, and arm interrupt
#endif
  //EnableInterrupts();
  StartOS();
}

//******** OS_AddSW1Task *************** 
// add a background task to run whenever the SW1 (PF4) button is pushed
// Inputs: pointer to a void/void background function
//         priority 0 is the highest, 5 is the lowest
// Outputs: 1 if successful, 0 if this thread can not be added
// It is assumed that the user task will run to completion and return
// This task can not spin, block, loop, sleep, or kill
// This task can call OS_Signal  OS_bSignal	 OS_AddThread
// This task does not have a Thread ID
// In labs 2 and 3, this command will be called 0 or 1 times
// In lab 2, the priority field can be ignored
// In lab 3, there will be up to four background threads, and this priority field 
//           determines the relative priority of these four threads
//
void (*SW1GlobalTask)(void);
long SW1GlobalTaskPriority;
int OS_AddSW1Task(void(*task)(void), unsigned long priority) {
    Board_Init();             // initialize PF0 and PF4 and make them inputs
    SW1GlobalTask = task; 
    SW1GlobalTaskPriority = priority; 
    return 0;
}

/**
 * @brief this function takes care of the GPIOPortF_Handler. 
 * specific implementation for Lab2: calls that function that is was request once the osADDSW1Task was called
 */
void GPIOPortF_Handler(void) {
    GPIO_PORTF_ICR_R = 0x01; //acknowlegement
    //OS_AddThread(SW1GlobalTask, STACKSIZE, SW1GlobalTaskPriority); //Note that the thread will kill itself after implemention (thus make sure the the function has OS_Kill in it)
		SW1GlobalTask();
    //fputc('o', stdout); //for debugging
}


/**
 * @brief runs a task periodically (based on what the TA said, they don't want us to add a thread
 */
void Timer0A_Handler(void) {
    TIMER0_ICR_R = TIMER_ICR_TATOCINT ;   //clearing the interrupt 
    GlobalPeriodicThread();
}

void SysTick_Handler(void){
    /* 
    Tcb_t* runningThread =  TCB_GetRunningThread();
    Tcb_t* possibleSleepingThread = runningThread;
    // decrementing threads with state_sleep > 1 
    if (possibleSleepingThread != NULL) { //enter ony if there is at least one thread 
        do {
            if (possibleSleepingThread->state_sleep > 1) {
                (possibleSleepingThread->state_sleep) -= 1 ;
            }
            possibleSleepingThread = possibleSleepingThread ->next;
        }while(possibleSleepingThread != runningThread);
    } 
    */
    long status;
    status = StartCritical();
		//DisableInterrupts();
    TCB_UpdateSleeping();
		//EnableInterrupts();
    EndCritical(status);
    //Context_Switch();
    NVIC_INT_CTRL_R = NVIC_INT_CTRL_PEND_SV;
    Time++;
}


unsigned long OS_Time() {
    return Time;
}

unsigned long OS_TimeDifference(unsigned long start, unsigned long stop) {
    return stop - start;
}





