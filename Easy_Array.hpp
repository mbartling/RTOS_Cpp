#ifndef __EASY_ARRAY_HPP__
#define __EASY_ARRAY_HPP__

template<typename T>
class EasyArray{
public:
  EasyArray(T* inRef, int inSize): storage(inRef), sz(inSize) {}

  template<typename Q> operator const EasyArray<const Q>()
  {
    //check implicit conversion of elements
    static_cast<Q>(*static_cast<T*>(0));
    //Cast EasyArray
    return EasyArray<const Q>(reinterpret_cast<Q*>(storage), sz);
  }
  T& operator[](int n){return storage[n];}
  const T& operator[](int n) const {return storage[n];}

  bool assign(EasyArray that){
    if(that.sz != sz){
      return false;
    }
    for(int i = 0; i < sz; ++i){
      storage[i] = that.storage[i];
    }
  }
  void reset(EasyArray that){
    reset(that.storage, that.sz);
  }
  void reset(T* inStorage, int inSz){
    storage = inStorage;
    sz = inSize;
  }

  int size(void) const {
    return sz;
  }
  //EasyArray does not own any resources. it uses
  // reference semantics

private:
  T* storage;
  int sz;
};

// Some helper functions for dealing with uninitialized areas
template<typename T> EasyArray<T> make_ref(T* inStorage, inSize)
{
  return (inStorage) ? EasyArray<T>(inStorage, inSize) : EasyArray<T>(0,0);
} 

// For built in arrays which compiler knows size
template<typename T, int s> EasyArray<T> make_ref(T (&pp)[s])
{
  return EasyArray<T>(pp,s);
}
#endif /* __EASY_ARRAY_HPP__*/
