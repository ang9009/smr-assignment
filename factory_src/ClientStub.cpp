#include "ClientStub.h"
#include "CustomerRequestType.h"
#include "Messages.h"
#include <stdexcept>

ClientStub::ClientStub() {}

int ClientStub::Init(std::string ip, int port) { return socket.Init(ip, port); }

void ClientStub::SendIDMessage() {
  IDMessage msg(IDMessageType::CLIENT);
  int size = msg.Size();
  char buffer[size];

  msg.Marshal(buffer);
  if (!socket.Send(buffer, size, 0)) {
    throw std::runtime_error("Failed to send ID message");
  }
}

RobotInfo ClientStub::Order(CustomerRequest req) {
  if (req.GetRequestType() != CustomerRequestType::ORDER) {
    throw std::runtime_error("Request should be of type order");
  }

  RobotInfo info;
  char buffer[32];
  int size = req.Size();

  req.Marshal(buffer);
  int sent = socket.Send(buffer, size, 0);
  if (!sent) {
    throw std::runtime_error("Failed to send complete order request to server");
  }

  size = info.Size();
  int recvd = socket.Recv(buffer, size, 0);
  if (!recvd) {
    throw std::runtime_error(
        "Failed to receive complete robot info from server");
  }

  info.Unmarshal(buffer);
  return info;
}

CustomerRecord ClientStub::ReadRecord(CustomerRequest req) {
  if (req.GetRequestType() != CustomerRequestType::READ) {
    throw std::runtime_error("Request should be of type read");
  }

  int req_size = req.Size();
  char req_buf[req_size];
  req.Marshal(req_buf);
  int sent = socket.Send(req_buf, req_size, 0);
  if (!sent) {
    throw std::runtime_error("Failed to send complete read request to server");
  }

  CustomerRecord record;
  int resp_size = record.Size();
  char resp_buf[32];
  int recvd = socket.Recv(resp_buf, resp_size, 0);
  if (!recvd) {
    throw std::runtime_error(
        "Failed to receive complete customer record from server");
  }

  record.Unmarshal(resp_buf);
  return record;
}