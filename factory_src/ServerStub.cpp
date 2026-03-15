#include "ServerStub.h"
#include "Messages.h"
#include <stdexcept>

ServerStub::ServerStub() {}

void ServerStub::Init(std::unique_ptr<ServerSocket> socket) {
  this->socket = std::move(socket);
}

PrimaryRequest ServerStub::ReceivePrimaryRequest() {
  PrimaryRequest req;
  char buffer[req.Size()];
  if (!socket->Recv(buffer, req.Size(), 0)) {
    throw std::runtime_error("Failed to receive primary request");
  }
  req.Unmarshal(buffer);
  return req;
}

void ServerStub::AckReplicationComplete() {
  ReplicationAck ack;
  char buffer[ack.Size()];
  ack.Marshal(buffer);
  socket->Send(buffer, ack.Size(), 0);
}

IDMessage ServerStub::ReceiveIDRequest() {
  IDMessage msg;
  char buffer[msg.Size()];
  if (socket->Recv(buffer, msg.Size(), 0)) {
    msg.Unmarshal(buffer);
  }
  return msg;
}

CustomerRequest ServerStub::ReceiveCustomerRequest() {
  char buffer[32];
  CustomerRequest request;
  if (socket->Recv(buffer, request.Size(), 0)) {
    request.Unmarshal(buffer);
  }
  return request;
}

int ServerStub::ShipRobot(RobotInfo info) {
  char buffer[32];
  info.Marshal(buffer);
  return socket->Send(buffer, info.Size(), 0);
}

int ServerStub::ReturnRecord(CustomerRecord record) {
  char buffer[32];
  record.Marshal(buffer);
  return socket->Send(buffer, record.Size(), 0);
}