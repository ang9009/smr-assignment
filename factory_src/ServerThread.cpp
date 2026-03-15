#include <cstdio>
#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>

#include "CustomerRequestType.h"
#include "MapOp.h"
#include "Messages.h"
#include "PrimaryClientStub.h"
#include "ServerInfo.h"
#include "ServerStub.h"
#include "ServerThread.h"

RobotFactory::RobotFactory(int id)
    : last_index(0), committed_index(-1), primary_id(-1), factory_id(id) {}

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

void RobotFactory::HandleCustomerRequest(ServerStub &stub,
                                         CustomerRequest &order,
                                         int engineer_id) {
  int request_type = order.GetRequestType();
  printf(
      "Received order from customer id: %d, request type: %d, order id: %d\n",
      order.GetCustomerId(), request_type, order.GetOrderNumber());

  switch (request_type) {
  case CustomerRequestType::ORDER: {
    RobotInfo robot = CreateRegularRobot(order, engineer_id);
    stub.ShipRobot(robot);
    break;
  }
  case CustomerRequestType::READ: {
    CustomerRecord record = GetCustomerRecord(order.GetCustomerId());
    stub.ReturnRecord(record);
    break;
  }
  default:
    std::cout << "Undefined request type: " << request_type << std::endl;
  }
}

void RobotFactory::HandlePrimaryRequest(const ServerStub &stub,
                                        const PrimaryRequest &req) {
  int incoming_id = req.GetFactoryId();
  if (primary_id != incoming_id) {
    primary_id = incoming_id;
  }
  printf("Received primary request from factory %d\n", primary_id);

  // Update log
  MapOp op = req.GetMapOp();
  int primary_last_index = req.GetLastIndex();
  if (primary_last_index < 0) {
    throw std::runtime_error("Received invalid last_index from primary: " +
                             std::to_string(primary_last_index));
  }
  if (smr_log.size() < static_cast<unsigned>(primary_last_index + 1)) {
    smr_log.resize(primary_last_index + 1);
  }
  smr_log[primary_last_index] = op;
  last_index = primary_last_index;

  // Commit index to log
  int new_committed_idx = req.GetCommittedIndex();
  if (new_committed_idx < 0 ||
      static_cast<unsigned>(new_committed_idx) >= smr_log.size()) {
    throw std::runtime_error("Received invalid committed_index from primary: " +
                             std::to_string(new_committed_idx));
  }
  MapOp committed_op = smr_log[new_committed_idx];
  switch (committed_op.opcode) {
  case Opcode::UPDATE_MAP: {
    std::lock_guard<std::mutex> lg(record_lock);
    customer_record[committed_op.arg1] = committed_op.arg2;
    break;
  }
  default:
    fprintf(stderr, "Got unrecognized opcode %d\n", committed_op.opcode);
    break;
  }
  committed_index = new_committed_idx;
  printf("Committed index %d\n", committed_index);
}

void RobotFactory::EngineerThread(std::unique_ptr<ServerSocket> socket,
                                  int id) {
  int engineer_id = id;
  CustomerRequest customer_req;
  PrimaryRequest primary_req;
  ServerStub stub;

  stub.Init(std::move(socket));
  // Determine thread type
  IDMessage id_msg = stub.ReceiveIDRequest();
  IDMessageType type = id_msg.GetType();
  printf("Received ID request, type %d\n", static_cast<int>(type));

  // Handle requests based on thread type
  while (true) {
    if (type == IDMessageType::CLIENT) {
      // This is the engineer thread
      customer_req = stub.ReceiveCustomerRequest();
      if (!customer_req.IsValid()) {
        break;
      }
      HandleCustomerRequest(stub, customer_req, engineer_id);
    } else if (type == IDMessageType::PRIMARY) {
      // This is the IFA thread
      try {
        primary_req = stub.ReceivePrimaryRequest();
        HandlePrimaryRequest(stub, primary_req);
        stub.AckReplicationComplete();
      } catch (std::exception &err) {
        printf("Failed to handle primary request: %s\n", err.what());
        break;
      }
    } else {
      printf("Received unknown request type %d\n", static_cast<int>(type));
      break;
    }
  }
}

// PFA thread
void RobotFactory::AdminThread(int id, std::vector<ServerInfo> backups) {
  std::unique_lock<std::mutex> arq_ul(arq_lock, std::defer_lock);
  PrimaryClientStub stub(std::move(backups));

  while (true) {
    arq_ul.lock();

    arq_cv.wait(arq_ul, [this] { return !arq.empty(); });

    auto req = std::move(arq.front());
    arq.pop();

    arq_ul.unlock();

    if (primary_id != factory_id) {
      primary_id = factory_id;
    }

    try {
      stub.ConnectToBackups();
    } catch (std::exception &err) {
      printf("Failed to connect to backups: %s\n", err.what());
      continue;
    }

    RobotInfo robot = req->robot;
    MapOp op = {.opcode = Opcode::UPDATE_MAP,
                .arg1 = robot.GetCustomerId(),
                .arg2 = robot.GetOrderNumber()};
    if (last_index >= smr_log.size()) {
      smr_log.resize(last_index + 1);
    }
    smr_log[last_index] = op;
    uint32_t written_index = last_index;
    last_index++;

    PrimaryRequest replic_req(op, factory_id, committed_index,
                              static_cast<int>(written_index));
    try {
      stub.SendReplicationRequests(replic_req);
    } catch (std::exception &err) {
      printf("Failed to send replication requests: %s\n", err.what());
      continue;
    }

    committed_index = static_cast<int>(written_index);
    {
      std::lock_guard<std::mutex> lg(record_lock);
      MapOp &applied = smr_log[written_index];
      if (applied.opcode == Opcode::UPDATE_MAP) {
        customer_record[applied.arg1] = applied.arg2;
      }
    }

    std::this_thread::sleep_for(std::chrono::microseconds(100));
    robot.SetAdminId(id);
    req->prom.set_value(robot);
  }
}
