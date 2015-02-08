#ifndef __POOL_HPP__
#define __POOL_HPP__

#include <stdint.h>
#include <cstddef>
template <class T, int N> class Pool
{
private:
  T storage[N];
  int8_t status[N];
  int numUsed;
public:
  Pool(void) : numUsed(0){
    for(int i = 0; i < N; i++){
      status[i] = 0;
    }
  }


  T* get(void){
    int nextAvail;

    for(int i = 0; i < N; i++){
      if(status[i] == 0){
        nextAvail = i;
        break;
      }
    }

    T* res = &storage[nextAvail];
    status[nextAvail] = 1;
     
    numUsed++;
    return res;
  }

  void free(T* elem){

    for(int i = 0; i < N; i++){
      if(elem == &storage[i]){
        status[i] = 0;
        numUsed--;
        elem = NULL;
        break;
      }
    }
    return;
  }
  int available(void) const{
    return N - numUsed;
  }

};

#endif /*__POOL_HPP__*/
