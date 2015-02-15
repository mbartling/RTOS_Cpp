#include "os.h"
#include "TCB.h"
#include "PLL.h"
#include "UART0.h"
#include <stdint.h>
#include "SysTickInts.h"
#include "inc/tm4c123gh6pm.h"

#ifdef __cplusplus
extern "C" {
#endif

// #define SYSTICK_EN 1

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode
void StartOS(void);
void ContextSwitch(void);
// void PendSV_Handler(void);

#ifdef __cplusplus
 }
#endif

int32_t ThreadCount = 0;

/** ******** OS_Init ************
 * @brief initialize operating system, disable interrupts until OS_Launch
 * @details initialize OS controlled I/O: serial, ADC, systick, LaunchPad 
 * I/O and timers 
 * 
 * @param input:  none
 * @return output: none
 */
void OS_Init(void)
{
  DisableInterrupts();
  PLL_Init();
#ifdef SYSTICK_EN
  SysTick_Init(160000); //2 Ms period default
#endif
  UART0_Init();

}



/** ******** OS_AddThread *************** 
 * @brief add a foregound thread to the scheduler
 * Inputs: pointer to a void/void foreground task
 *         number of bytes allocated for its stack
 *         priority, 0 is highest, 5 is the lowest
 * Outputs: 1 if successful, 0 if this thread can not be added
 * stack size must be divisable by 8 (aligned to double word boundary)
 * In Lab 2, you can ignore both the stackSize and priority fields
 * In Lab 3, you can ignore the stackSize fields
 */

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



/** ******** OS_Suspend ************
 * @brief suspend execution of currently running thread
 * @details scheduler will choose another thread to execute
 * Can be used to implement cooperative multitasking 
 * Same function as OS_Sleep(0)
 *
 * @param input:  none
 * @return output: none
 */
void OS_Suspend(void)
{
  //This is where we would do any scheduling

  // NVIC_INT_CTRL_R = 0x10000000; //Trigger PendSV
  NVIC_INT_CTRL_R = NVIC_INT_CTRL_PEND_SV;
}

/** ******** OS_Launch *************** 
 * @brief start the scheduler, enable interrupts
 * @details Inputs: number of 12.5ns clock cycles for each time slice
 *         you may select the units of this parameter
 * Outputs: none (does not return)
 * In Lab 2, you can ignore the theTimeSlice field
 * In Lab 3, you should implement the user-defined TimeSlice field
 * It is ok to limit the range of theTimeSlice to match the 24-bit SysTick
 * @param input:  none
 * @return output: none
 */
void OS_Launch(unsigned long theTimeSlice)
{
#ifdef SYSTICK_EN
  NVIC_ST_RELOAD_R = theTimeSlice - 1;
  NVIC_ST_CTRL_R = 0x00000007;  //Enable core clock, and arm interrupt
#endif
  EnableInterrupts();
  StartOS();
}

// void SysTick_Handler(void){
  
//   ContextSwitch();
// }
// void PendSV_Handler(void){
//   ContextSwitch();
// }