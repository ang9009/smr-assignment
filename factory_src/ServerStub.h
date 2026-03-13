#ifndef __SERVER_STUB_H__
#define __SERVER_STUB_H__

#include <memory>

#include "Messages.h"
#include "ServerSocket.h"

class ServerStub {
private:
  std::unique_ptr<ServerSocket> socket;

public:
  ServerStub();
  void Init(std::unique_ptr<ServerSocket> socket);
  CustomerRequest ReceiveRequest();
  int ShipRobot(RobotInfo info);
  int ReturnRecord(CustomerRecord record);
};

#endif // end of #ifndef __SERVER_STUB_H__
