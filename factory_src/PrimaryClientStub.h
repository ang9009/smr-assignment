#pragma once

#include "Messages.h"
#include "ServerInfo.h"
#include <vector>

// Client stub used by the primary factory admin (PFA)
class PrimaryClientStub {
private:
  std::vector<ServerInfo> backups; // Backup nodes info

public:
  PrimaryClientStub(std::vector<ServerInfo> backups);
  bool ConnectToBackup(ServerInfo &backup);
  void SendIDMessage();
  void SendReplicationRequests(const PrimaryRequest &req);
  void ReconnectToBackups(const std::vector<MapOp> &smr_log, int primary_id,
                          int committed_index);
};