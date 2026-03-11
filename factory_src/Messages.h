#ifndef __MESSAGES_H__
#define __MESSAGES_H__

#include <string>

class RobotOrder {
private:
	int customer_id;
	int order_number;
	int robot_type;

public:
	RobotOrder();
	void operator = (const RobotOrder &order) {
		customer_id = order.customer_id;
		order_number = order.order_number;
		robot_type = order.robot_type;
	}
	void SetOrder(int cid, int order_num, int type);
	int GetCustomerId();
	int GetOrderNumber();
	int GetRobotType();

	int Size();

	void Marshal(char *buffer);
	void Unmarshal(char *buffer);

	bool IsValid();

	void Print();
};

class RobotInfo {
private:
	int customer_id;
	int order_number;
	int robot_type;
	int engineer_id;
	int expert_id;

public:
	RobotInfo();
	void operator = (const RobotInfo &info) {
		customer_id = info.customer_id;
		order_number = info.order_number;
		robot_type = info.robot_type;
		engineer_id = info.engineer_id;
		expert_id = info.expert_id;
	}
	void SetInfo(int cid, int order_num, int type, int engid, int expid);
	void CopyOrder(RobotOrder order);
	void SetEngineerId(int id);
	void SetExpertId(int id);

	int GetCustomerId();
	int GetOrderNumber();
	int GetRobotType();
	int GetEngineerId();
	int GetExpertId();

	int Size();

	void Marshal(char *buffer);
	void Unmarshal(char *buffer);

	bool IsValid();

	void Print();
};

#endif // #ifndef __MESSAGES_H__
