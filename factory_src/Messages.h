#ifndef __MESSAGES_H__
#define __MESSAGES_H__

#include "MapOp.h"
#include <string>

// Identification message type; tells the idle factory admin who is connecting
enum class IDMessageType : int {
  PRIMARY = 1, // sender is the primary factory
  CLIENT = 2,  // sender is a client
  UNSET = -1
};

class IDMessage {
public:
  IDMessage();
  IDMessage(IDMessageType type);

  IDMessageType GetType() const;

  int Size() const;
  void Marshal(char *buffer);
  void Unmarshal(char *buffer);

private:
  IDMessageType type;
};

class PrimaryRequest {
private:
  MapOp op;
  int factory_id;      // ID of the primary
  int committed_index; // last committed index from primary
  int last_index;      // last log index from primary

public:
  PrimaryRequest();
  PrimaryRequest(MapOp op, int factory_id, int committed_index, int last_index);

  MapOp GetMapOp() const;
  int GetFactoryId() const;
  int GetCommittedIndex() const;
  int GetLastIndex() const;
  int Size() const;

  void Marshal(char *buffer) const;
  void Unmarshal(char *buffer);
};

class ReplicationAck {
public:
  ReplicationAck();
  int Size() const;
  void Marshal(char *buffer);
  void Unmarshal(char *buffer);

private:
  int status; // 1 = success
};

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
