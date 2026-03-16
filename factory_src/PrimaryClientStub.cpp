#include "PrimaryClientStub.h"
#include "MapOp.h"
#include "Messages.h"
#include <stdexcept>

PrimaryClientStub::PrimaryClientStub(std::vector<ServerInfo> backups)
    : backups(backups) {}

void PrimaryClientStub::SendIDMessage() {
  IDMessage msg(IDMessageType::PRIMARY);
  int size = msg.Size();
  char buffer[size];

  msg.Marshal(buffer);
  for (ServerInfo &backup : backups) {
    if (!backup.socket.Send(buffer, size, 0)) {
      throw std::runtime_error("Failed to send ID message");
    }
  }
}

bool PrimaryClientStub::ConnectToBackup(ServerInfo &backup) {
  IDMessage msg(IDMessageType::PRIMARY);
  int id_size = msg.Size();
  char id_buf[id_size];
  msg.Marshal(id_buf);

  if (backup.socket.IsInitialized()) {
    return true;
  }

  // Ignore backup failures
  int init = backup.socket.Init(backup.ip, backup.port);
  int sent = backup.socket.Send(id_buf, id_size, 0);
  return init && sent;
}

void PrimaryClientStub::ReconnectToBackups(const std::vector<MapOp> &smr_log,
                                           int primary_id,
                                           int committed_index) {
  if (committed_index < 0 ||
      static_cast<size_t>(committed_index) >= smr_log.size()) {
    // Nothing to send or invalid
    return;
  }

  ReplicationAck ack;
  int ack_size = ack.Size();
  char ack_buf[ack_size];

  for (ServerInfo &backup : backups) {
    if (backup.socket.IsInitialized()) {
      continue;
    }

    bool connected = ConnectToBackup(backup);
    if (!connected) {
      continue;
    }

    // Send all existing log entries one by one so repaired server catches up
    for (int i = 0; i <= committed_index; i++) {
      PrimaryRequest req(smr_log[i], primary_id, i, i);
      int size = req.Size();
      char buffer[size];
      req.Marshal(buffer);

      if (!backup.socket.Send(buffer, size, 0) ||
          !backup.socket.Recv(ack_buf, ack_size, 0)) {
        backup.socket.Close();
        break;
      }
    }
  }
}

void PrimaryClientStub::SendReplicationRequests(const PrimaryRequest &req) {
  int size = req.Size();
  char buffer[size];
  req.Marshal(buffer);

  ReplicationAck ack;
  int ack_size = ack.Size();
  char ack_buf[ack_size];

  for (ServerInfo &backup : backups) {
    if (!backup.socket.IsInitialized()) {
      continue;
    }

    int sent = backup.socket.Send(buffer, size, 0);
    int acked = backup.socket.Recv(ack_buf, ack_size, 0);

    if (!sent || !acked) {
      backup.socket.Close();
    }
  }
}