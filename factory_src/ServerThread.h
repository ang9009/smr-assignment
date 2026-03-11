#ifndef __SERVERTHREAD_H__
#define __SERVERTHREAD_H__

#include <condition_variable>
#include <future>
#include <mutex>
#include <queue>
#include <thread>

#include "Messages.h"
#include "ServerSocket.h"

struct ExpertRequest {
  RobotInfo robot;
  std::promise<RobotInfo> prom;
};

class RobotFactory {
private:
  std::queue<std::unique_ptr<ExpertRequest>> erq;
  std::mutex erq_lock;
  std::condition_variable erq_cv;

  RobotInfo CreateRegularRobot(RobotOrder order, int engineer_id);
  RobotInfo CreateSpecialRobot(RobotOrder order, int engineer_id);

public:
  void EngineerThread(std::unique_ptr<ServerSocket> socket, int id);
  void AdminThread(int id);
};

#endif // end of #ifndef __SERVERTHREAD_H__
