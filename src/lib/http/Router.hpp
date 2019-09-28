#pragma once

#include <vector>
#include <regex>
#include <string>

#include "Request.hpp"
#include "Response.hpp"

namespace HTTP {

bool default_404 (Request *req, Response *res) {
  res->setContentType("text/plain");
  res->code = 404;
  res->write("404 Error");

  return true;
}

bool default_500 (Request *req, Response *res) {
  res->setContentType("text/plain");
  res->code = 500;
  res->write("500 Error");

  return true;
}

struct Route {
  Route (std::regex path, uint8_t method, bool (*func)(Request *, Response *)) {
    this->path = path;
    this->method = method;
    this->func = func;
  }

  std::regex path;
  uint8_t method;
  bool (*func)(Request *, Response *);
};

class Router {
public:
  Router () {

  }

  Route *addRoute (std::regex path, uint8_t method, bool (*func)(Request *, Response *)) {
    Route *route = new Route(path, method, func);

    routes.push_back(route);

    return route;
  }

  void set500 (bool (*func)(Request *, Response *)) {
    fivehundred = func;
  }

  void set404 (bool (*func)(Request *, Response *)) {
    fourohfour = func;
  }

  uint16_t numRoutes () {
    return routes.size();
  }

#ifdef ARCH_WIN
  bool route (Request *request, SOCKET fd) {
#else
  bool route (Request *request, int fd) {
#endif
    std::string uri;
    if (request->uri) {
      uri = request->uri;
    }

    bool done = false;
    Response *response = new Response(fd);

    if (request->method == ERROR || request->uri == nullptr) {
      if (fivehundred) {
        (*fivehundred)(request, response);
      } else {
        default_500(request, response);
      }

      return false;
    }

    for (std::vector<Route *>::iterator it = routes.begin(); it != routes.end() && !done; it++) {
      Route *r = *it;

      if (request->method == r->method && std::regex_match(uri, r->path)) {
        if (r->func) {
          done = r->func(request, response);
        }
      }
    }

    if (!done) {
      if (fourohfour) {
        (*fourohfour)(request, response);
      } else {
        default_404(request, response);
      }
    }

    response->write(nullptr);

    delete response;

    return done;
  }

private:
  std::vector<Route *> routes;
  bool (*fourohfour)(Request *, Response *) = { 0 };
  bool (*fivehundred)(Request *, Response *) = { 0 };
};


};
