#pragma once

#include "ClientSocket.h"
#include <string>

struct ServerInfo {
  int id;
  std::string ip;
  int port;
  ClientSocket socket;
};