#pragma once

#include <string>

#include "Request.hpp"
#include "Router.hpp"

extern "C" {
#ifdef ARCH_WIN
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
}

namespace HTTP {

class Server {
public:
  Server (uint16_t port) {
#ifdef ARCH_WIN
    WSADATA wsaData;
    char portStr[256];
    sprintf(portStr, "%d", port);

    int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
      error = "WSAStartup failed";
      return;
    }

    struct addrinfo *result = NULL, hints;

    ZeroMemory(&hints, sizeof (hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the local address and port to be used by the server
    iResult = getaddrinfo(NULL, portStr, &hints, &result);
    if (iResult != 0) {
      error = "getaddrinfo failed";
      WSACleanup();
      return;
    }

    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

    // Setup the TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
      error = "bind failed";
      freeaddrinfo(result);
      closesocket(ListenSocket);
      WSACleanup();
      return;
    }

    freeaddrinfo(result);

    if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR ) {
      error = "unable to listen to socket";
      closesocket(ListenSocket);
      WSACleanup();
      return;
    }

#else
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
#endif

    isRunning = true;
  }

  Route *addRoute(std::regex path, uint8_t method, bool (*func)(Request *, Response *)) {
    return router.addRoute(path, method, func);
  }

  void run () {
#ifndef ARCH_WIN
    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(cli_addr);
#endif
    char buffer[40960];

    while (1) {
#ifdef ARCH_WIN
      SOCKET newsockfd = INVALID_SOCKET;

      // Accept a client socket
      newsockfd = accept(ListenSocket, NULL, NULL);
      if (newsockfd == INVALID_SOCKET) {
        perror("ERROOR on accept");
        continue;
      }

      recv(newsockfd, buffer, 40960, 0);
      Request *req = new Request(buffer);
      router.route(req, newsockfd);
      closesocket(newsockfd);
#else
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
#endif
    }
  }

  bool isRunning;
  std::string error;

private:
#ifdef ARCH_WIN
  WSADATA wsaData;
  SOCKET ListenSocket;
#else
  int fd;
#endif
  Router router;
};

};
