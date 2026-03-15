#include "ClientThread.h"
#include "CustomerRequestType.h"
#include "Messages.h"

#include <cstdio>
#include <exception>

ClientThreadClass::ClientThreadClass() {}

int ClientThreadClass::GetTotalOrdered(int customer_id) {
  CustomerRecord record = ReadRecord(customer_id);
  int last_order = record.GetLastOrder(); // zero-indexed order num
  return last_order >= 0 ? last_order + 1 : 0;
}

bool ClientThreadClass::DoOrder(int customer_id, int order_num) {
  RobotInfo robot;

  timer.Start();

  CustomerRequest order_req;
  order_req.SetRequest(customer_id, order_num, CustomerRequestType::ORDER);
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

CustomerRecord ClientThreadClass::ReadRecord(int customer_id) {
  CustomerRequest req;
  req.SetRequest(customer_id, -1, CustomerRequestType::READ);
  return stub.ReadRecord(req);
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

  try {
    stub.SendIDMessage();
  } catch (std::exception &err) {
    printf("Failed to send ID message: %s\n", err.what());
    return;
  }

  for (int i = 0; i < num_orders; i++) {
    if (request_type == CustomerRequestType::ORDER) {
      int total_ordered = 0;
      try {
        total_ordered = GetTotalOrdered(customer_id);
      } catch (std::exception &err) {
        printf("Failed get total num of orders: %s\n", err.what());
        return;
      }
      int order_num = total_ordered + i;
      // printf("Order number: %d\n", order_num);
      if (!DoOrder(customer_id, order_num)) {
        break;
      }
    } else if (request_type == CustomerRequestType::READ) {
      try {
        ReadRecord(customer_id).Print();
      } catch (std::exception &err) {
        printf("Request failed: %s\n", err.what());
        break;
      }
    } else if (request_type == CustomerRequestType::READ_ALL) {
      int cid = i; // In this case, i is the customer id
      try {
        CustomerRecord record = ReadRecord(cid);
        if (record.GetCustomerId() != -1) {
          printf("%d\t%d\n", record.GetCustomerId(), record.GetLastOrder());
        }
      } catch (std::exception &err) {
        printf("Request failed: %s\n", err.what());
        break;
      }
    } else {
      printf("Request type %d not recognized\n", request_type);
    }
  }
}

ClientTimer ClientThreadClass::GetTimer() { return timer; }
