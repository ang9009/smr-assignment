#include "Messages.h"
#include "MapOp.h"
#include <arpa/inet.h>
#include <cstdio>
#include <cstring>

PrimaryRequest::PrimaryRequest()
    : op{}, factory_id(-1), committed_index(-1), last_index(-1) {}

PrimaryRequest::PrimaryRequest(MapOp op, int factory_id, int committed_index,
                               int last_index)
    : op(op), factory_id(factory_id), committed_index(committed_index),
      last_index(last_index) {}

MapOp PrimaryRequest::GetMapOp() const { return op; }

int PrimaryRequest::GetFactoryId() const { return factory_id; }

int PrimaryRequest::GetCommittedIndex() const { return committed_index; }

int PrimaryRequest::GetLastIndex() const { return last_index; }

int PrimaryRequest::Size() const { return 6 * sizeof(int); }

void PrimaryRequest::Marshal(char *buffer) const {
  int net_opcode = htonl(op.opcode);
  int net_arg1 = htonl(op.arg1);
  int net_arg2 = htonl(op.arg2);
  int net_factory_id = htonl(factory_id);
  int net_committed_index = htonl(committed_index);
  int net_last_index = htonl(last_index);
  int offset = 0;

  memcpy(buffer + offset, &net_opcode, sizeof(net_opcode));
  offset += sizeof(net_opcode);
  memcpy(buffer + offset, &net_arg1, sizeof(net_arg1));
  offset += sizeof(net_arg1);
  memcpy(buffer + offset, &net_arg2, sizeof(net_arg2));
  offset += sizeof(net_arg2);
  memcpy(buffer + offset, &net_factory_id, sizeof(net_factory_id));
  offset += sizeof(net_factory_id);
  memcpy(buffer + offset, &net_committed_index, sizeof(net_committed_index));
  offset += sizeof(net_committed_index);
  memcpy(buffer + offset, &net_last_index, sizeof(net_last_index));
}

void PrimaryRequest::Unmarshal(char *buffer) {
  int offset = 0;
  int net_opcode, net_arg1, net_arg2, net_factory_id, net_committed_index,
      net_last_index;

  memcpy(&net_opcode, buffer + offset, sizeof(net_opcode));
  offset += sizeof(net_opcode);
  memcpy(&net_arg1, buffer + offset, sizeof(net_arg1));
  offset += sizeof(net_arg1);
  memcpy(&net_arg2, buffer + offset, sizeof(net_arg2));
  offset += sizeof(net_arg2);
  memcpy(&net_factory_id, buffer + offset, sizeof(net_factory_id));
  offset += sizeof(net_factory_id);
  memcpy(&net_committed_index, buffer + offset, sizeof(net_committed_index));
  offset += sizeof(net_committed_index);
  memcpy(&net_last_index, buffer + offset, sizeof(net_last_index));

  op.opcode = static_cast<Opcode>(ntohl(net_opcode));
  op.arg1 = ntohl(net_arg1);
  op.arg2 = ntohl(net_arg2);
  factory_id = ntohl(net_factory_id);
  committed_index = ntohl(net_committed_index);
  last_index = ntohl(net_last_index);
}

ReplicationAck::ReplicationAck() : status(1) {}

int ReplicationAck::Size() const { return sizeof(status); }

void ReplicationAck::Marshal(char *buffer) {
  int net_status = htonl(status);
  memcpy(buffer, &net_status, sizeof(net_status));
}

void ReplicationAck::Unmarshal(char *buffer) {
  int net_status;
  memcpy(&net_status, buffer, sizeof(net_status));
  status = ntohl(net_status);
}

IDMessage::IDMessage() : type(IDMessageType::UNSET) {}

IDMessage::IDMessage(IDMessageType type) : type(type) {}

IDMessageType IDMessage::GetType() const { return type; }

int IDMessage::Size() const { return sizeof(int); }

void IDMessage::Marshal(char *buffer) {
  int value = static_cast<int>(type);
  int net_value = htonl(value);
  memcpy(buffer, &net_value, sizeof(net_value));
}

void IDMessage::Unmarshal(char *buffer) {
  int net_value;
  memcpy(&net_value, buffer, sizeof(net_value));
  type = static_cast<IDMessageType>(ntohl(net_value));
}

CustomerRequest::CustomerRequest() {
  customer_id = -1;
  order_number = -1;
  request_type = -1;
}

void CustomerRequest::SetRequest(int id, int number, int type) {
  customer_id = id;
  order_number = number;
  request_type = type;
}

int CustomerRequest::GetCustomerId() { return customer_id; }
int CustomerRequest::GetOrderNumber() { return order_number; }
int CustomerRequest::GetRequestType() { return request_type; }

int CustomerRequest::Size() {
  return sizeof(customer_id) + sizeof(order_number) + sizeof(request_type);
}

void CustomerRequest::Marshal(char *buffer) {
  int net_customer_id = htonl(customer_id);
  int net_order_number = htonl(order_number);
  int net_robot_type = htonl(request_type);
  int offset = 0;
  memcpy(buffer + offset, &net_customer_id, sizeof(net_customer_id));
  offset += sizeof(net_customer_id);
  memcpy(buffer + offset, &net_order_number, sizeof(net_order_number));
  offset += sizeof(net_order_number);
  memcpy(buffer + offset, &net_robot_type, sizeof(net_robot_type));
}

void CustomerRequest::Unmarshal(char *buffer) {
  int net_customer_id;
  int net_order_number;
  int net_robot_type;
  int offset = 0;
  memcpy(&net_customer_id, buffer + offset, sizeof(net_customer_id));
  offset += sizeof(net_customer_id);
  memcpy(&net_order_number, buffer + offset, sizeof(net_order_number));
  offset += sizeof(net_order_number);
  memcpy(&net_robot_type, buffer + offset, sizeof(net_robot_type));

  customer_id = ntohl(net_customer_id);
  order_number = ntohl(net_order_number);
  request_type = ntohl(net_robot_type);
}

bool CustomerRequest::IsValid() { return (customer_id != -1); }

void CustomerRequest::Print() {
  printf("id %d num %d type %d\n", customer_id, order_number, request_type);
}

RobotInfo::RobotInfo() {
  customer_id = -1;
  order_number = -1;
  robot_type = -1;
  engineer_id = -1;
  admin_id = -1;
}

void RobotInfo::SetInfo(int id, int number, int type, int engid, int expid) {
  customer_id = id;
  order_number = number;
  robot_type = type;
  engineer_id = engid;
  admin_id = expid;
}

void RobotInfo::CopyOrder(CustomerRequest order) {
  customer_id = order.GetCustomerId();
  order_number = order.GetOrderNumber();
  robot_type = order.GetRequestType();
}
void RobotInfo::SetEngineerId(int id) { engineer_id = id; }
void RobotInfo::SetAdminId(int id) { admin_id = id; }

int RobotInfo::GetCustomerId() { return customer_id; }
int RobotInfo::GetOrderNumber() { return order_number; }
int RobotInfo::GetRobotType() { return robot_type; }
int RobotInfo::GetEngineerId() { return engineer_id; }
int RobotInfo::GetAdminId() { return admin_id; }

int RobotInfo::Size() {
  return sizeof(customer_id) + sizeof(order_number) + sizeof(robot_type) +
         sizeof(engineer_id) + sizeof(admin_id);
}

void RobotInfo::Marshal(char *buffer) {
  int net_customer_id = htonl(customer_id);
  int net_order_number = htonl(order_number);
  int net_robot_type = htonl(robot_type);
  int net_engineer_id = htonl(engineer_id);
  int net_admin_id = htonl(admin_id);
  int offset = 0;

  memcpy(buffer + offset, &net_customer_id, sizeof(net_customer_id));
  offset += sizeof(net_customer_id);
  memcpy(buffer + offset, &net_order_number, sizeof(net_order_number));
  offset += sizeof(net_order_number);
  memcpy(buffer + offset, &net_robot_type, sizeof(net_robot_type));
  offset += sizeof(net_robot_type);
  memcpy(buffer + offset, &net_engineer_id, sizeof(net_engineer_id));
  offset += sizeof(net_engineer_id);
  memcpy(buffer + offset, &net_admin_id, sizeof(net_admin_id));
}

void RobotInfo::Unmarshal(char *buffer) {
  int net_customer_id;
  int net_order_number;
  int net_robot_type;
  int net_engineer_id;
  int net_admin_id;
  int offset = 0;

  memcpy(&net_customer_id, buffer + offset, sizeof(net_customer_id));
  offset += sizeof(net_customer_id);
  memcpy(&net_order_number, buffer + offset, sizeof(net_order_number));
  offset += sizeof(net_order_number);
  memcpy(&net_robot_type, buffer + offset, sizeof(net_robot_type));
  offset += sizeof(net_robot_type);
  memcpy(&net_engineer_id, buffer + offset, sizeof(net_engineer_id));
  offset += sizeof(net_engineer_id);
  memcpy(&net_admin_id, buffer + offset, sizeof(net_admin_id));

  customer_id = ntohl(net_customer_id);
  order_number = ntohl(net_order_number);
  robot_type = ntohl(net_robot_type);
  engineer_id = ntohl(net_engineer_id);
  admin_id = ntohl(net_admin_id);
}

bool RobotInfo::IsValid() { return (customer_id != -1); }

void RobotInfo::Print() {
  printf("id %d num %d type %d engid %d admin_id %d\n", customer_id,
         order_number, robot_type, engineer_id, admin_id);
}

CustomerRecord::CustomerRecord() : customer_id(-1), last_order(-1) {}

CustomerRecord::CustomerRecord(int cid, int last_order_num)
    : customer_id(cid), last_order(last_order_num) {}

void CustomerRecord::SetRecord(int cid, int order) {
  customer_id = cid;
  last_order = order;
}

int CustomerRecord::GetCustomerId() { return customer_id; }
int CustomerRecord::GetLastOrder() { return last_order; }

int CustomerRecord::Size() { return sizeof(customer_id) + sizeof(last_order); }

void CustomerRecord::Marshal(char *buffer) {
  int net_customer_id = htonl(customer_id);
  int net_last_order = htonl(last_order);
  int offset = 0;
  memcpy(buffer + offset, &net_customer_id, sizeof(net_customer_id));
  offset += sizeof(net_customer_id);
  memcpy(buffer + offset, &net_last_order, sizeof(net_last_order));
}

void CustomerRecord::Unmarshal(char *buffer) {
  int net_customer_id;
  int net_last_order;
  int offset = 0;
  memcpy(&net_customer_id, buffer + offset, sizeof(net_customer_id));
  offset += sizeof(net_customer_id);
  memcpy(&net_last_order, buffer + offset, sizeof(net_last_order));
  customer_id = ntohl(net_customer_id);
  last_order = ntohl(net_last_order);
}

void CustomerRecord::Print() {
  printf("customer_id %d last_order %d\n", customer_id, last_order);
}
