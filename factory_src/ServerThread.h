#ifndef __SERVERTHREAD_H__
#define __SERVERTHREAD_H__

#include <condition_variable>
#include <future>
#include <map>
#include <mutex>
#include <queue>
#include <thread>

#include "Messages.h"
#include "ServerSocket.h"

struct AdminRequest {
  RobotInfo robot;
  std::promise<RobotInfo> prom;
};

struct MapOp {
  int opcode; // operation code : 1 - update value
  int arg1;   // customer_id to apply the operation
  int arg2;   // parameter for the operation
};

class RobotFactory {
private:
  std::queue<std::unique_ptr<AdminRequest>> arq;
  std::mutex arq_lock;
  std::condition_variable arq_cv;
  std::map<int, int> customer_record;
  std::mutex record_lock;
  std::vector<MapOp> smr_log;
  std::mutex smr_lock;

  RobotInfo CreateRegularRobot(CustomerRequest order, int engineer_id);
  CustomerRecord GetCustomerRecord(int customer_id);
  // RobotInfo CreateSpecialRobot(CustomerRequest order, int engineer_id);

public:
  void EngineerThread(std::unique_ptr<ServerSocket> socket, int id);
  void AdminThread(int id);
};

#endif // end of #ifndef __SERVERTHREAD_H__
