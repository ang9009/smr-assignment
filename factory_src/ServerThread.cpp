#include <iostream>
#include <iterator>
#include <memory>

#include "Messages.h"
#include "ServerStub.h"
#include "ServerThread.h"

// RobotInfo RobotFactory::CreateRegularRobot(CustomerRequest order,
//                                            int engineer_id) {
//   RobotInfo robot;
//   robot.CopyOrder(order);
//   robot.SetEngineerId(engineer_id);
//   robot.SetAdminId(-1);
//   return robot;
// }

RobotInfo RobotFactory::CreateRegularRobot(CustomerRequest order,
                                           int engineer_id) {
  RobotInfo robot;
  robot.CopyOrder(order);
  robot.SetEngineerId(engineer_id);

  std::promise<RobotInfo> prom;
  std::future<RobotInfo> fut = prom.get_future();

  std::unique_ptr<AdminRequest> req =
      std::unique_ptr<AdminRequest>(new AdminRequest);
  req->robot = robot;
  req->prom = std::move(prom);

  // Notify admin
  arq_lock.lock();
  arq.push(std::move(req));
  arq_cv.notify_one();
  arq_lock.unlock();

  robot = fut.get();
  return robot;
}

CustomerRecord RobotFactory::GetCustomerRecord(int customer_id) {
  int cid, last_order_num;
  {
    std::lock_guard<std::mutex> lg(record_lock);
    std::map<int, int>::iterator record_entry =
        customer_record.find(customer_id);
    if (record_entry != customer_record.end()) {
      cid = record_entry->first;
      last_order_num = record_entry->second;
    } else {
      cid = -1;
      last_order_num = -1;
    }
  }

  return CustomerRecord(cid, last_order_num);
}

void RobotFactory::EngineerThread(std::unique_ptr<ServerSocket> socket,
                                  int id) {
  int engineer_id = id;
  int request_type;
  CustomerRequest order;
  RobotInfo robot;
  CustomerRecord record;

  ServerStub stub;

  stub.Init(std::move(socket));

  while (true) {
    order = stub.ReceiveRequest();
    if (!order.IsValid()) {
      break;
    }

    request_type = order.GetRequestType();
    // TODO: remove later
    std::cout << "Received order from customer id: " << order.GetCustomerId()
              << ", request type: " << request_type
              << ", order id: " << order.GetOrderNumber();

    // If CustomerRequest has other info, print here.
    std::cout << std::endl;
    switch (request_type) {
    case 1:
      robot = CreateRegularRobot(order, engineer_id);
      stub.ShipRobot(robot);
      break;
    case 2:
      record = GetCustomerRecord(order.GetCustomerId());
      stub.ReturnRecord(record);
      break;
    default:
      std::cout << "Undefined request type: " << request_type << std::endl;
    }
  }
}

void RobotFactory::AdminThread(int id) {
  std::unique_lock<std::mutex> arq_ul(arq_lock, std::defer_lock);

  while (true) {
    arq_ul.lock();

    arq_cv.wait(arq_ul, [this] { return !arq.empty(); });

    auto req = std::move(arq.front());
    arq.pop();

    arq_ul.unlock();

    RobotInfo robot = req->robot;

    // Update log
    {
      std::lock_guard<std::mutex> lg(smr_lock);
      MapOp op = {.opcode = 1,
                  .arg1 = robot.GetCustomerId(),
                  .arg2 = robot.GetOrderNumber()};
      smr_log.push_back(op);
    }

    // Update customer record map
    {
      std::lock_guard<std::mutex> lg(record_lock);
      customer_record[robot.GetCustomerId()] = robot.GetOrderNumber();
    }

    std::this_thread::sleep_for(std::chrono::microseconds(100));
    robot.SetAdminId(id);
    req->prom.set_value(robot);
  }
}
