#ifndef __SERVERTHREAD_H__
#define __SERVERTHREAD_H__

#include <condition_variable>
#include <future>
#include <map>
#include <mutex>
#include <queue>
#include <thread>

#include "MapOp.h"
#include "ServerInfo.h"
#include "ServerSocket.h"
#include "ServerStub.h"

struct AdminRequest {
  RobotInfo robot;
  std::promise<RobotInfo> prom;
};

class RobotFactory {
private:
  std::queue<std::unique_ptr<AdminRequest>> arq;
  std::mutex arq_lock;
  std::condition_variable arq_cv;
  std::map<int, int> customer_record;
  std::mutex record_lock;
  std::vector<MapOp> smr_log; // only admin (PFA) or IFA thread writes
  unsigned last_index;        // the last index of the smr_log that has data
  int committed_index; // the last index of the smr_log where the MapOp of the
                       // log entry is committed and applied to the
                       // customer_record. Initially set to -1.
  int primary_id; // the production factory id ( server id ). Initially set to
                  // -1. There's only 1 admin thread updating this, so a mutex
                  // is not necessary.
  const int factory_id; // the id of the factory. This is assigned via the
                        // command line arguments.

  RobotInfo CreateRegularRobot(CustomerRequest order, int engineer_id);
  CustomerRecord GetCustomerRecord(int customer_id);
  void HandleCustomerRequest(ServerStub &stub, CustomerRequest &order,
                             int engineer_id);
  // RobotInfo CreateSpecialRobot(CustomerRequest order, int engineer_id);
  void HandlePrimaryRequest(const ServerStub &stub, const PrimaryRequest &req);

public:
  // TODO: continue from here, initialize all new properties + write
  // constructor
  RobotFactory(int id);
  void EngineerThread(std::unique_ptr<ServerSocket> socket, int id);
  void AdminThread(int id, std::vector<ServerInfo> backups);
};

#endif // end of #ifndef __SERVERTHREAD_H__
