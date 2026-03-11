#include <cstring>
#include <iostream>

#include <arpa/inet.h>
#include "Messages.h"

RobotOrder::RobotOrder() {
	customer_id = -1;
	order_number = -1;
	robot_type = -1;
}

void RobotOrder::SetOrder(int id, int number, int type) {
	customer_id = id;
	order_number = number;
	robot_type = type;
}

int RobotOrder::GetCustomerId() { return customer_id; }
int RobotOrder::GetOrderNumber() { return order_number; }
int RobotOrder::GetRobotType() { return robot_type; }

int RobotOrder::Size() {
	return sizeof(customer_id) + sizeof(order_number) + sizeof(robot_type);
}

void RobotOrder::Marshal(char *buffer) {
	int net_customer_id = htonl(customer_id);
	int net_order_number = htonl(order_number);
	int net_robot_type = htonl(robot_type);
	int offset = 0;
	memcpy(buffer + offset, &net_customer_id, sizeof(net_customer_id));
	offset += sizeof(net_customer_id);
	memcpy(buffer + offset, &net_order_number, sizeof(net_order_number));
	offset += sizeof(net_order_number);
	memcpy(buffer + offset, &net_robot_type, sizeof(net_robot_type));
}

void RobotOrder::Unmarshal(char *buffer) {
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
	robot_type = ntohl(net_robot_type);
}

bool RobotOrder::IsValid() {
	return (customer_id != -1);
}

void RobotOrder::Print() {
	std::cout << "id " << customer_id << " ";
	std::cout << "num " << order_number << " ";
	std::cout << "type " << robot_type << std::endl;
}

RobotInfo::RobotInfo() {
	customer_id = -1;
	order_number = -1;
	robot_type = -1;
	engineer_id = -1;
	expert_id = -1;
}

void RobotInfo::SetInfo(int id, int number, int type, int engid, int expid) {
	customer_id = id;
	order_number = number;
	robot_type = type;
	engineer_id = engid;
	expert_id = expid;
}

void RobotInfo::CopyOrder(RobotOrder order) {
	customer_id = order.GetCustomerId();
	order_number = order.GetOrderNumber();
	robot_type = order.GetRobotType();
}
void RobotInfo::SetEngineerId(int id) { engineer_id = id; }
void RobotInfo::SetExpertId(int id) { expert_id = id; }

int RobotInfo::GetCustomerId() { return customer_id; }
int RobotInfo::GetOrderNumber() { return order_number; }
int RobotInfo::GetRobotType() { return robot_type; }
int RobotInfo::GetEngineerId() { return engineer_id; }
int RobotInfo::GetExpertId() { return expert_id; }

int RobotInfo::Size() {
	return sizeof(customer_id) + sizeof(order_number) + sizeof(robot_type)
		+ sizeof(engineer_id) + sizeof(expert_id);
}

void RobotInfo::Marshal(char *buffer) {
	int net_customer_id = htonl(customer_id);
	int net_order_number = htonl(order_number);
	int net_robot_type = htonl(robot_type);
	int net_engineer_id = htonl(engineer_id);
	int net_expert_id = htonl(expert_id);
	int offset = 0;

	memcpy(buffer + offset, &net_customer_id, sizeof(net_customer_id));
	offset += sizeof(net_customer_id);
	memcpy(buffer + offset, &net_order_number, sizeof(net_order_number));
	offset += sizeof(net_order_number);
	memcpy(buffer + offset, &net_robot_type, sizeof(net_robot_type));
	offset += sizeof(net_robot_type);
	memcpy(buffer + offset, &net_engineer_id, sizeof(net_engineer_id));
	offset += sizeof(net_engineer_id);
	memcpy(buffer + offset, &net_expert_id, sizeof(net_expert_id));

}

void RobotInfo::Unmarshal(char *buffer) {
	int net_customer_id;
	int net_order_number;
	int net_robot_type;
	int net_engineer_id;
	int net_expert_id;
	int offset = 0;

	memcpy(&net_customer_id, buffer + offset, sizeof(net_customer_id));
	offset += sizeof(net_customer_id);
	memcpy(&net_order_number, buffer + offset, sizeof(net_order_number));
	offset += sizeof(net_order_number);
	memcpy(&net_robot_type, buffer + offset, sizeof(net_robot_type));
	offset += sizeof(net_robot_type);
	memcpy(&net_engineer_id, buffer + offset, sizeof(net_engineer_id));
	offset += sizeof(net_engineer_id);
	memcpy(&net_expert_id, buffer + offset, sizeof(net_expert_id));

	customer_id = ntohl(net_customer_id);
	order_number = ntohl(net_order_number);
	robot_type = ntohl(net_robot_type);
	engineer_id = ntohl(net_engineer_id);
	expert_id = ntohl(net_expert_id);
}

bool RobotInfo::IsValid() {
	return (customer_id != -1);
}

void RobotInfo::Print() {
	std::cout << "id " << customer_id << " ";
	std::cout << "num " << order_number << " ";
	std::cout << "type " << robot_type << " ";
	std::cout << "engid " << engineer_id << " ";
	std::cout << "expid " << expert_id << std::endl;
}

