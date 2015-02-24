#ifndef __MAILBOX_HPP__
#define __MAILBOX_HPP__
#include "os.h"

template <class T>
class Mailbox{
private:
  Sema4Type semaSend;
  Sema4Type semaAck;
  T Mail;
public:

  inline void Send(T& data){
    Mail = data;
    OS_Signal(&semaSend);
    OS_Wait(&semaAck);
  }
  inline void Receive(T& data){
    OS_Wait(&semaSend);
    data = Mail;
    OS_Signal(&semaAck);
  }

};
#endif /*__MAILBOX_HPP__*/