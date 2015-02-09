#ifndef __TCB_H__
#define __TCB_H__

#include <stdint.h>

#define MAXNUMTHREADS 3  /** Maximum number of threads*/
#define STACKSIZE 100 /** number of 32bit words in stack */

typedef struct _Tcb {
  int32_t* sp;        //!< Stack Pointer
                      //!< Valid for threads not running        
  struct _Tcb* next;  //!< Next TCB Element
  struct _Tcb* prev;  //!< Previous TCB element
  int32_t id;         //!< Thread ID
  int32_t state_sleep;//!< used to suspend execution
  uint32_t priority;   //!< Thread priority
  int32_t state_blocked; //!<Used in lab 3
  int32_t stack[STACKSIZE]; //!<Thread stack
} Tcb_t;

/**
 * @brief Doubly circular list
 * @details Modified from Martin Broadhursts online method
 * 
 */
typedef struct _TcbListC
{
  Tcb_t* head;
  uint32_t count;  
} TcbListC_t;

//Tcb_t TcbTable[MAXNUMTHREADS];

void TCB_SetInitialStack(Tcb_t* pTcb);
Tcb_t* TCB_InsertNode(Tcb_t* root);
int TCB_Available(void);
Tcb_t* TCB_GetNewThread(void);
void TCB_InsertNodeBeforeRoot(Tcb_t* node);
Tcb_t* TCB_GetRunningThread(void);

//void dummy(void); // Tests if function pointer set properly
#endif /*__TCB_H__*/
