#ifndef __CLIENT_STUB_H__
#define __CLIENT_STUB_H__

#include <string>

#include "ClientSocket.h"
#include "Messages.h"

class ClientStub {
private:
	ClientSocket socket;
public:
	ClientStub();
	int Init(std::string ip, int port);
	RobotInfo OrderRobot(RobotOrder order);
};


#endif // end of #ifndef __CLIENT_STUB_H__
