#pragma once

#include "Request.hpp"
#include <cstdint>
#include <cstdlib>
#include <cstring>
#ifdef ARCH_WIN
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#endif

namespace HTTP {

class Response {
public:
#ifdef ARCH_WIN
  Response (SOCKET fd) {
#else
  Response (int fd) {
#endif
    this->fd = fd;
  }

  ~Response () {
    for (uint8_t i = 0; i < num_headers; i++) {
      free(headers[i]);
    }

    if (content_type) {
      free(content_type);
    }
  }

  void setHeader (char *header) {
    if (num_headers < MAX_HEADERS - 1) {
      headers[num_headers] = strdup(header);
      num_headers++;
    }
  }

  void setContentType (const char *type) {
    content_type = strdup(type);
  }

  size_t write (const char *data) {
    if (!headers_sent) {
      sendHeaders();
    }

    if (data == nullptr) {
      return 0;
    }

    return _write(data);
  }

  size_t write (void *data, size_t length) {
    if (!headers_sent) {
      sendHeaders();
    }

#ifdef ARCH_WIN
    return send(fd, (char *) data, length, 0);
#else
    return send(fd, data, length, 0);
#endif
  }

  uint16_t code = 0;

private:
  bool headers_sent = false;
  char *headers[MAX_HEADERS] = { 0 };
  uint8_t num_headers = 0;
  char *content_type = nullptr;
#ifdef ARCH_WIN
  SOCKET fd;
#else
  int fd;
#endif

  const char *responseStatus () {
    switch (code) {
      case 200:
      return "HTTP/1.1 200 OK\r\n";
      case 404:
      return "HTTP/1.1 404 NOT FOUND\r\n";
      case 500:
      return "HTTP/1.1 500 ERROR\r\n";
      default:
      return "HTTP/1.1 200 OK\r\n";
    }
  }

  void sendHeaders () {
    if (code == 0) {
      code = 200;
    }

    headers_sent = true;

    _write(responseStatus());
    _write("Content-type: ");

    if (content_type != nullptr && content_type[0] != '\0') {
      _write(content_type);
      _write("\r\n");
    } else {
      _write("text/html\r\n");
    }

    for (uint8_t i = 0; i < num_headers; i++) {
      _write(headers[i]);
      _write("\r\n");
    }

    _write("\r\n");
  }

  size_t _write (const char *data) {
    size_t len = strlen(data);

#ifdef ARCH_WIN
    return send(fd, (char *) data, len, 0);
#else
    return send(fd, (void *) data, len, 0);
#endif
  }

};


};
