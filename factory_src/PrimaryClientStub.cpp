#include "PrimaryClientStub.h"
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

void PrimaryClientStub::ConnectToBackups() {
  IDMessage msg(IDMessageType::PRIMARY);
  int id_size = msg.Size();
  char id_buf[id_size];
  msg.Marshal(id_buf);

  for (ServerInfo &backup : backups) {
    if (backup.socket.IsInitialized()) {
      continue;
    }

    if (!backup.socket.Init(backup.ip, backup.port)) {
      throw std::runtime_error("Failed to connect to all backups");
    }
    if (!backup.socket.Send(id_buf, id_size, 0)) {
      throw std::runtime_error("Failed to send ID message to backup");
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
      std::string msg = "Socket to backup with id " +
                        std::to_string(backup.id) + " not initialized";
      throw std::runtime_error(msg);
    }
    if (!backup.socket.Send(buffer, size, 0)) {
      throw std::runtime_error("Failed to send replication request to backup");
    }
    if (!backup.socket.Recv(ack_buf, ack_size, 0)) {
      throw std::runtime_error("Did not receive replication ack from backup");
    }
    ack.Unmarshal(ack_buf);
  }
}