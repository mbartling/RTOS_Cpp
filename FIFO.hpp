#ifndef __FIFO_HPP__
#define __FIFO_HPP__

#ifdef __cplusplus
extern "C" {
#endif

long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value

#ifdef __cplusplus
 }
#endif

template <typename T>
 class Fifo{
protected:
  enum Status {FAIL=-1, SUCCESS=0};

 public:
  virtual int Put(T data){
    return FAIL; 
  }
  virtual int Get(T* data){
    return (FAIL);
  }
  virtual unsigned short getSize(void){
    return (unsigned short) 0;
  }
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
  T FifoData[Size];
  

public:
  FifoP(){
    long sr;
    sr = StartCritical();
    PutPt = GetPt = &FifoData[0];
    EndCritical(sr);
  }

  int Put(T data){
    T volatile *nextPutPt;
    nextPutPt = PutPt + 1;

    if(nextPutPt == &FifoData[Size]){
      nextPutPt = &FifoData[0];
    }
    if(nextPutPt == GetPt){
      return(FAIL);
    }
    else{
      *(PutPt) = data;
      PutPt = nextPutPt;
      return(SUCCESS);
    }
  }

  int Get(T *data){
    if(PutPt = GetPt){
      return(FAIL);
    }
    *data = *(GetPt++);
    if(GetPt == &FifoData[Size]){
      GetPt = &FifoData[0];
    }
    return SUCCESS;
  }

  unsigned short getSize(void){
    if(PutPt < GetPt){
      return (unsigned short) (PutPt - GetPt + Size*sizeof(T))/sizeof(T);
    }
    return (unsigned short)(PutPt - GetPt)/sizeof(T);
  }
};

#endif /*__FIFO_HPP__*/