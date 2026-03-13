#include "ClientThread.h"
#include "Messages.h"

#include <cstdio>

ClientThreadClass::ClientThreadClass() {}

int ClientThreadClass::GetTotalOrdered(int customer_id) {
  CustomerRecord record;
  CustomerRequest read_req;
  read_req.SetRequest(customer_id, -1, RequestType::READ);
  try {
    record = stub.ReadRecord(read_req);
    record.Print();
  } catch (std::exception &err) {
    printf("Request failed: %s\n", err.what());
    return -1;
  }

  int last_order = record.GetLastOrder(); // zero-indexed order num
  return last_order >= 0 ? last_order + 1 : 0;
}

bool ClientThreadClass::DoOrder(int customer_id, int order_num) {
  RobotInfo robot;

  timer.Start();

  CustomerRequest order_req;
  order_req.SetRequest(customer_id, order_num, RequestType::ORDER);
  try {
    robot = stub.Order(order_req);
  } catch (std::exception &err) {
    printf("Request failed: %s\n", err.what());
    return false;
  }
  robot.Print();
  timer.EndAndMerge();

  if (!robot.IsValid()) {
    printf("Invalid robot %d\n", customer_id);
    return false;
  }

  return true;
}

bool ClientThreadClass::DoReadRecord(int customer_id) {
  CustomerRequest req;
  int order_num = -1; // order num isn't necessary here
  req.SetRequest(customer_id, order_num, RequestType::READ);

  timer.Start();
  try {
    CustomerRecord record = stub.ReadRecord(req);
    record.Print();
  } catch (std::exception &err) {
    printf("Request failed: %s\n", err.what());
    return false;
  }
  timer.EndAndMerge();

  return true;
}

void ClientThreadClass::ThreadBody(std::string ip, int port, int id, int orders,
                                   int type) {
  customer_id = id;
  num_orders = orders;
  request_type = type;
  if (!stub.Init(ip, port)) {
    printf("Thread %d failed to connect\n", customer_id);
    return;
  }

  // Read current latest order number
  // ! NOTE: if clients are allowed to make orders for each other, this will
  // ! break due to race conditions with the latest order number
  int total_ordered = GetTotalOrdered(customer_id);
  if (total_ordered == -1) {
    return;
  }

  for (int i = 0; i < num_orders; i++) {
    if (request_type == RequestType::ORDER) {
      int order_num = total_ordered + i;
      // printf("Order number: %d\n", order_num);
      if (!DoOrder(customer_id, order_num)) {
        break;
      }
    } else if (request_type == RequestType::READ) {
      if (!DoReadRecord(customer_id)) {
        break;
      }
    } else if (request_type == RequestType::READ_ALL) {
      int cid = i; // In this case, i is the customer id
      if (!DoReadRecord(cid)) {
        break;
      }
    } else {
      printf("Request type %d not recognized\n", request_type);
    }
  }
}

ClientTimer ClientThreadClass::GetTimer() { return timer; }
