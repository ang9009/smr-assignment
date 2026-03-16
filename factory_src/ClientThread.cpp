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
    return false;
  }
  timer.EndAndMerge();

  if (!robot.IsValid()) {
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
    return;
  }

  try {
    stub.SendIDMessage();
  } catch (std::exception &err) {
    return;
  }

  for (int i = 0; i < num_orders; i++) {
    if (request_type == CustomerRequestType::ORDER) {
      int total_ordered = 0;
      try {
        total_ordered = GetTotalOrdered(customer_id);
      } catch (std::exception &err) {
        return;
      }
      int order_num = total_ordered + i;
      if (!DoOrder(customer_id, order_num)) {
        return;
      }
    } else if (request_type == CustomerRequestType::READ) {
      try {
        (void)ReadRecord(customer_id);
      } catch (std::exception &err) {
        return;
      }
    } else if (request_type == CustomerRequestType::READ_ALL) {
      int cid = i; // In this case, i is the customer id
      try {
        CustomerRecord record = ReadRecord(cid);
        if (record.GetCustomerId() != -1) {
          printf("%d\t%d\n", record.GetCustomerId(), record.GetLastOrder());
        }
      } catch (std::exception &err) {
        return;
      }
    }
  }
}

ClientTimer ClientThreadClass::GetTimer() { return timer; }
