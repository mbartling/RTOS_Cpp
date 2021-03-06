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
#include "priority.h"
#include "Mailbox.hpp"
#define STACKSIZE 100
#define SYSTICK_EN 1  
#include "FIFO.hpp"
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
unsigned long systemTime= 0;
unsigned long systemTime2= 0;
volatile unsigned long systemTime3= 0;
volatile unsigned long systemPeriod = 0;
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
int Timer1APeriod = TIME_1MS/1000; // .1us 

void OS_Init(void)
{
  DisableInterrupts();
  PLL_Init();
  NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R& ~NVIC_SYS_PRI3_PENDSV_M)|PendSVPriority; // priority 6
#ifdef SYSTICK_EN
  SysTick_Init(160000); //2 Ms period default
#endif
//  Timer1A_Init((uint32_t)Timer1APeriod);
  UART0_Init();
  //EnableInterrupts(); 
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
    Timer2A_Init((uint32_t)period);
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
		
    //	long status = StartCritical();
//    Tcb_t * runningThread = TCB_GetRunningThread();
//    runningThread-> state_sleep = 0;
//    TCB_RemoveRunningAndSleep();
    //If running list empty then insert idle task
//	  EndCritical(status);
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
void OS_Launch(unsigned long theTimeSlice){
    systemPeriod = theTimeSlice;
#ifdef SYSTICK_EN

    NVIC_ST_RELOAD_R = theTimeSlice - 1;
    //	NVIC_ST_CURRENT_R = 0;      // any write to current clears it
    NVIC_ST_CTRL_R = 0x00000007;  //Enable core clock, and arm interrupt
#endif
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

void (*SW2GlobalTask)(void);
long SW2GlobalTaskPriority;
int OS_AddSW2Task(void(*task)(void), unsigned long priority) {
    Board_Init();             // initialize PF0 and PF4 and make them inputs
    SW2GlobalTask = task; 
    SW2GlobalTaskPriority = priority; 
    return 0;
}
/**
 * @brief this function takes care of the GPIOPortF_Handler. 
 * specific implementation for Lab2: calls that function that is was request once the osADDSW1Task was called
 */
void GPIOPortF_Handler(void) {
    if (GPIO_PORTF_RIS_R&0x10) {
      GPIO_PORTF_ICR_R = 0x10; //acknowlegement
      SW1GlobalTask();
    }else if (GPIO_PORTF_RIS_R&0x1){
      GPIO_PORTF_ICR_R = 0x1; //acknowlegement
      SW2GlobalTask();
    }
}


/**
 * @brief runs a task periodically (based on what the TA said, they don't want us to add a thread
 */
//void Timer0A_Handler(void) {
//    systemTime2++;
//    TIMER0_ICR_R = TIMER_ICR_TATOCINT ;   //clearing the interrupt 
//    GlobalPeriodicThread();
//}

/**
 * @brief runs a task periodically (based on what the TA said, they don't want us to add a thread
 */
void Timer2A_Handler(void) {
    systemTime2++;
    TIMER2_ICR_R = TIMER_ICR_TATOCINT ;   //clearing the interrupt 
    GlobalPeriodicThread();
}
/**
 * @brief used for measuring the time (this timer is used in OS_Time();
 */
void Timer1A_Handler(void) {
   TIMER1_ICR_R = TIMER_ICR_TATOCINT ;   //clearing the interrupt 
   systemTime++; 
}

void SysTick_Handler(void){
    long status;
    status = StartCritical();
    //DisableInterrupts();
    TCB_UpdateSleeping();
    //EnableInterrupts();
    EndCritical(status);
    Context_Switch();
    //NVIC_INT_CTRL_R = NVIC_INT_CTRL_PEND_SV;
    systemTime3++;
}


unsigned long OS_Time() {
    return systemTime3*(systemPeriod) + systemPeriod - NVIC_ST_CURRENT_R;
   // return systemTime* (Timer1APeriod);
}

unsigned long OS_TimeDifference(unsigned long start, unsigned long stop) {
    return stop - start;
}

 /********* OS_ClearMsTime ************
 * sets the system time to zero (from Lab 1)
 * Inputs:  none
 * Outputs: none
 * You are free to change how this works
 */
void OS_ClearMsTime(void) {
    systemTime3 = 0;
}

 /********* OS_MsTime ************
 * reads the current time in msec (from Lab 1)
 * Inputs:  none
 * Outputs: time in ms units
 * You are free to select the time resolution for this function
 * It is ok to make the resolution to match the first call to OS_AddPeriodicThread
 */
unsigned long OS_MsTime(void) {
    return  (systemTime3*(systemPeriod) + systemPeriod - NVIC_ST_CURRENT_R)/TIME_1MS;
}

 /********* OS_Fifo_Init ************
 * Initialize the Fifo to be empty
 * Inputs: size
 * Outputs: none 
 * In Lab 2, you can ignore the size field
 * In Lab 3, you should implement the user-defined fifo size
 * In Lab 3, you can put whatever restrictions you want on size
 *    e.g., 4 to 64 elements
 *    e.g., must be a power of 2,4,8,16,32,64,128
 */
#define FIFOSIZE 32 
FifoP<unsigned long , FIFOSIZE> OS_Fifo;
void OS_Fifo_Init(unsigned long size) {
}

 /********* OS_Fifo_Put ************
 * Enter one data sample into the Fifo
 * Called from the background, so no waiting 
 * Inputs:  data
 * Outputs: true if data is properly saved,
 *          false if data not saved, because it was full
 * Since this is called by interrupt handlers 
 *  this function can not disable or enable interrupts
 */

int OS_Fifo_Put(unsigned long data) {
    return OS_Fifo.Put(data) ? 1 : 0;
}
/********* OS_Fifo_Get ************
 * Remove one data sample from the Fifo
 * Called in foreground, will spin/block if empty
 * Inputs:  none
 * Outputs: data
 */

unsigned long OS_Fifo_Get(void) {
    unsigned long  data;
    OS_Fifo.Get(&data);
    return data;
}
/********* OS_Fifo_Size ************
 * Check the status of the Fifo
 * Inputs: none
 * Outputs: returns the number of elements in the Fifo
 *          greater than zero if a call to OS_Fifo_Get will return right away
 *          zero or less than zero if the Fifo is empty 
 *          zero or less than zero if a call to OS_Fifo_Get will spin or block
 */
long OS_Fifo_Size(void) {
    return OS_Fifo.getSize();
}

Mailbox<unsigned long> OSMailBox;
/********* OS_MailBox_Init ************
 * Initialize communication channel
 * Inputs:  none
 * Outputs: none
 */
void OS_MailBox_Init(void) {
    return;
}
 /********* OS_MailBox_Send ************
 * enter mail into the MailBox
 * Inputs:  data to be sent
 * Outputs: none
 * This function will be called from a foreground thread
 * It will spin/block if the MailBox contains data not yet received 
 */
void OS_MailBox_Send(unsigned long data) {
    OSMailBox.Send(data);
}
 /********* OS_MailBox_Recv ************
 * remove mail from the MailBox
 * Inputs:  none
 * Outputs: data received
 * This function will be called from a foreground thread
 * It will spin/block if the MailBox is empty 
 */
unsigned long OS_MailBox_Recv(void) {
    unsigned long data; 
    OSMailBox.Receive(data);
   return data; 
}

/**
 * @brief This function is used for stand alone testing, when we don't want to launch the os. It activates the PLL and UART so we can talk to the terminal and print values
 */
void OS_setupTest(void)
{
  DisableInterrupts();
  PLL_Init();
  UART0_Init();
  EnableInterrupts(); 
}




