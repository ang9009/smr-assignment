#ifndef __CLIENT_THREAD_H__
#define __CLIENT_THREAD_H__

#include <chrono>
#include <ctime>
#include <string>

#include "ClientStub.h"
#include "ClientTimer.h"

class ClientThreadClass {
  int customer_id;
  int num_orders;
  int request_type;
  ClientStub stub;
  ClientTimer timer;

  bool
  DoOrder(int customer_id,
          int order_num); // order_num should be the accumulative, zero-indexed
                          // order number, not the one in the current session
  bool DoReadRecord(int customer_id);
  int GetTotalOrdered(int customer_id); // returns -1 on order

public:
  ClientThreadClass();
  void ThreadBody(std::string ip, int port, int id, int orders, int type);
  void SendIDRequest();
  ClientTimer GetTimer();
};

#endif // end of #ifndef __CLIENT_THREAD_H__
