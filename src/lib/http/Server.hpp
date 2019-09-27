#pragma once

#include <string>

#include "Request.hpp"
#include "Router.hpp"

extern "C" {
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
}

namespace HTTP {

class Server {
public:
  Server (uint16_t port) {
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
      isRunning = false;
      error = "unable to create socket";
      return;
    }

    int enable = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
      isRunning = false;
      error = "setsockopt(SO_REUSEADDR) failed";
      return;
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(fd, (struct sockaddr *) &address, sizeof(address)) < 0) {
      isRunning = false;
      error = "unable to bind socket";
      return;
    }

    if (listen(fd, 1024) < 0) {
      isRunning = false;
      error = "unable to listen to socket";
      return;
    }

    isRunning = true;
  }

  Route *addRoute(std::regex path, uint8_t method, bool (*func)(Request *, Response *)) {
    return router.addRoute(path, method, func);
  }

  void run () {
    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(cli_addr);
    char buffer[40960];

    while (1) {
      /* Accept actual connection from the client */
      int newsockfd = accept(fd, (struct sockaddr *) &cli_addr, &clilen);

      if (newsockfd < 0) {
        perror("ERROR on accept");
        continue;
      }

      /* If connection is established then start communicating */
      bzero(buffer, 40960);
      size_t n = read(newsockfd, buffer, 40960);

      if (n < 0) {
        perror("ERROR reading from socket");
        close(newsockfd);
      }

      Request *req = new Request(buffer);
      router.route(req, newsockfd);
      close(newsockfd);
    }
  }

  bool isRunning;
  std::string error;

private:
  int fd;
  Router router;
};

};
