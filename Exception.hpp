#ifndef __EXCEPTION_HPP__
#define __EXCEPTION_HPP__

class Exception {
  int id;

public:
  Exception(int _id): id(_id) {}

  void assert(bool condition, bool expected){
    if(condition == expected){
      return;
    }
    else{
      ask_OS();
    }
  }//end assert

  //ask os for help
  void ask_OS(void){
    while(true){
        //Do nothing.
        //OS should handle it
      }
  }
  void fail(){
    ask_OS();
  }
  enum EXCEPTION_TYPES = { method_not_implemented,
                           NULLPTR_access,
                           memory_not_valid,
                           invalid_path,
                           invalid_initialization,
                           michael_too_awesome
                          }
};

Exception method_not_implemented(Exception::method_not_implemented);

#endif /*__EXCEPTION_HPP__*/