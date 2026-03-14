#include <iostream>
#include <set>
#include <stdexcept>
#include <thread>
#include <unordered_set>
#include <vector>

#include "ServerInfo.h"
#include "ServerSocket.h"
#include "ServerThread.h"

// Throws error if peers don't have unique id, or if number of peers doesn't
// match num_peers
std::vector<ServerInfo> GetPeersInfo(int num_peers, int my_id, char *argv[],
                                     int argc) {
  int expected_argc = 4 + 3 * num_peers;
  if (argc != expected_argc) {
    throw std::runtime_error("Missing peer info\n");
  }

  std::vector<ServerInfo> info;
  std::unordered_set<int> ids_set{my_id};
  for (int i = 4; i < argc; i += 3) {
    int id = atoi(argv[i]);
    if (ids_set.count(id)) {
      throw std::runtime_error("Found non-unique id " + std::to_string(id) +
                               " in args");
    }
    ids_set.insert(id);

    ServerInfo server_info = {
        .id = id, .ip = argv[i + 1], .port = atoi(argv[i + 2])};
    info.push_back(server_info);
  }

  return info;
}

int main(int argc, char *argv[]) {
  int port;
  int id;
  int num_peers;

  ServerSocket socket;
  RobotFactory factory;
  std::unique_ptr<ServerSocket> new_socket;
  std::vector<std::thread> thread_vector;

  if (argc < 4) {
    std::cout << "expected at least 3 arguments" << std::endl;
    std::cout << argv[0]
              << "[port #] [unique ID] [# peers] (repeat [ID] [IP] [port #])"
              << std::endl;
    return 1;
  }
  port = atoi(argv[1]);
  id = atoi(argv[2]);
  num_peers = atoi(argv[3]);

  try {
    std::vector<ServerInfo> peers = GetPeersInfo(num_peers, id, argv, argc);
  } catch (const std::exception &err) {
    printf("Error while getting peer info: %s\n", err.what());
    return 1;
  }

  // Initialize admin thread
  int num_admins = 1;
  for (int i = 0; i < num_admins; i++) {
    std::thread admin_thread(&RobotFactory::AdminThread, &factory, i);
    thread_vector.push_back(std::move(admin_thread));
  }

  if (!socket.Init(port)) {
    std::cout << "Socket initialization failed" << std::endl;
    return 1;
  }

  // Initialize engineer threads
  int engineer_cnt = 0;
  while ((new_socket = socket.Accept())) {
    std::thread engineer_thread(&RobotFactory::EngineerThread, &factory,
                                std::move(new_socket), engineer_cnt++);
    thread_vector.push_back(std::move(engineer_thread));
  }

  return 0;
}
