#include <iostream>

#include <arpa/inet.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "ServerSocket.h"

ServerSocket::ServerSocket(int fd, bool nagle_on) {
  fd_ = fd;
  is_initialized_ = true;
  NagleOn(nagle_on);
}

bool ServerSocket::Init(int port) {
  if (is_initialized_) {
    return true;
  }

  struct sockaddr_in addr;
  fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (fd_ < 0) {
    perror("ERROR: failed to create a socket");
    return false;
  }

  int reuse = 1;
  if (setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
    perror("ERROR: setsockopt SO_REUSEADDR failed");
    close(fd_);
    fd_ = -1;
    return false;
  }

  memset(&addr, '\0', sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(port);

  if ((bind(fd_, (struct sockaddr *)&addr, sizeof(addr))) < 0) {
    perror("ERROR: failed to bind");
    return false;
  }

  listen(fd_, 8);

  is_initialized_ = true;
  return true;
}

std::unique_ptr<ServerSocket> ServerSocket::Accept() {
  int accepted_fd;
  struct sockaddr_in addr;
  unsigned int addr_size = sizeof(addr);
  accepted_fd = accept(fd_, (struct sockaddr *)&addr, &addr_size);
  if (accepted_fd < 0) {
    perror("ERROR: failed to accept connection");
    return nullptr;
  }

  return std::unique_ptr<ServerSocket>(
      new ServerSocket(accepted_fd, IsNagleOn()));
}
