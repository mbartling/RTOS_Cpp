#ifndef __FIFO_HPP__
#define __FIFO_HPP__

#include "os.h"
#ifdef __cplusplus
extern "C" {
#endif

long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value

#ifdef __cplusplus
 }
#endif

#define SUCCESS true
#define FAIL false
	
template <typename T>
 class Fifo{
 	// enum Status {FAIL=-1, SUCCESS=0};
  Sema4Type m;
  Sema4Type available;

 public:
	 Fifo(void){
			OS_InitSemaphore(&m, 1);
		 OS_InitSemaphore(&available, 0);
	 }
  //using SUCCESS = true;
  //using FAIL = false;

 
//  virtual bool Put(T data){
//    return FAIL; 
//  }
//  virtual bool Get(T* data){
//    return (FAIL);
//  }
  inline void Wait(){
    OS_Wait(&available);
  }
  inline void bWait(){
    OS_Wait(&m);
  }
  inline void Signal(){
    OS_Signal(&available);
  }
  inline void bSignal(){
    OS_Signal(&m);
  }

//  virtual unsigned short getSize(void){
//    return (unsigned short) 0;
//  }
 };
/**
 * @brief Pointer type Fifo
 * @details [long description]
 * 
 * @tparam T [description]
 * @tparam Size [description]
 */
template <typename T, int Size>
class FifoP : public Fifo<T>{
  T volatile * PutPt;
  T volatile * GetPt;
  T volatile FifoData[Size];
  uint32_t LostData;
  // enum Status {FAIL=-1, SUCCESS=0};

public:
  FifoP(){
    long sr;
    sr = StartCritical();
    PutPt = GetPt = &FifoData[0];
    EndCritical(sr);
  }

  bool Put(T data){
    T volatile *nextPutPt;
    nextPutPt = PutPt + 1;

    if(nextPutPt == &FifoData[Size]){
      nextPutPt = &FifoData[0];
    }
    //if(nextPutPt == GetPt){
      // return(FAIL);
    
    // while(nextPutPt == GetPt){
      // this->Wait();
    // }
  //}
    if(nextPutPt == GetPt){
      LostData++;
    }
   else{
      *(PutPt) = data;
      PutPt = nextPutPt;
      this->Signal();
      return(SUCCESS);
      // return true;
    }
	 return(FAIL);
  }

  bool Get(T *data){
    // if(PutPt == GetPt){
      // return(FAIL);
    // }
    // while(GetPt == PutPt){
    this->Wait(); //Wait till available
    // }
    this->bWait();
    *data = *(GetPt++);
    if(GetPt == &FifoData[Size]){
      GetPt = &FifoData[0];
    }
    this->bSignal();
    // this->Signal();
    return SUCCESS;
  }

  unsigned short getSize(void){
    if(PutPt < GetPt){
      return (unsigned short) (PutPt - GetPt + Size*sizeof(T))/sizeof(T);
    }
    return (unsigned short)(PutPt - GetPt)/sizeof(T);
  }
};

template<typename T, int Size> 
class FifoI : public Fifo<T>{
  uint32_t volatile PutI;
  uint32_t volatile GetI;
  T FifoData[Size];
  // enum Status {FAIL=-1, SUCCESS=0};

public:
  FifoI(){
    long sr;
    sr = StartCritical();
    PutI = GetI = 0;
    EndCritical(sr);
  }

  bool Put(T data){
    if((PutI - GetI) & ~(Size - 1)){
      return FAIL;
    }
    FifoData[PutI & (Size - 1)] = data;
    PutI++;
    return SUCCESS;
  }

  bool Get(T* data){
    if(PutI == GetI){
      return(FAIL);
    }
    *data = FifoData[GetI & (Size - 1)];
    GetI++;
    return SUCCESS;
  }

  unsigned short getSize(void){
    return (unsigned short)(PutI - GetI);
  }

};
#endif /*__FIFO_HPP__*/
