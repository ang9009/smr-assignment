#ifndef __MESSAGES_H__
#define __MESSAGES_H__

#include <string>

enum RequestType : int { ORDER = 1, READ = 2, UNSET = -1 };

class CustomerRequest {
private:
  int customer_id;
  int order_number;
  int request_type; // Either 1 - regular robot order request , or
  // 2 - customer record read request

public:
  CustomerRequest();
  void operator=(const CustomerRequest &request) {
    customer_id = request.customer_id;
    order_number = request.order_number;
    request_type = request.request_type;
  }
  void SetRequest(int cid, int order_num, int type);
  int GetCustomerId();
  int GetOrderNumber();
  int GetRequestType();

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
  int admin_id;

public:
  RobotInfo();
  void operator=(const RobotInfo &info) {
    customer_id = info.customer_id;
    order_number = info.order_number;
    robot_type = info.robot_type;
    engineer_id = info.engineer_id;
    admin_id = info.admin_id;
  }
  void SetInfo(int cid, int order_num, int type, int engid, int expid);
  void CopyOrder(CustomerRequest order);
  void SetEngineerId(int id);
  void SetAdminId(int id);

  int GetCustomerId();
  int GetOrderNumber();
  int GetRobotType();
  int GetEngineerId();
  int GetAdminId();

  int Size();

  void Marshal(char *buffer);
  void Unmarshal(char *buffer);

  bool IsValid();

  void Print();
};

class CustomerRecord {
private:
  int customer_id; // -1 if not found in the map
  int last_order;  // -1 if not found in the map

public:
  CustomerRecord();
  CustomerRecord(int cid, int last_order_num);
  void SetRecord(int cid, int order);
  int GetCustomerId();
  int GetLastOrder();
  int Size();
  void Marshal(char *buffer);
  void Unmarshal(char *buffer);
  void Print();
};

#endif // #ifndef __MESSAGES_H__
