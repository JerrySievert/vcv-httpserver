#pragma once

#ifndef MAX_PARAMS
#define MAX_PARAMS 25
#endif

#ifndef MAX_HEADERS
#define MAX_HEADERS 25
#endif

namespace HTTP {
  enum Method {
    ERROR,
    GET,
    POST,
    PUT,
    DELETE
  };

class Request {
public:
  Request (char *request) {
    this->request = request;
    uint16_t position = 0;

    // start iterating through the request, break out the parts.
    // this is clunky, but fast
    if (request[0] == 'G' && request[1] == 'E' && request[2] == 'T' && request[3] == ' ') {
      position = 4;
      method = GET;
    } else if (request[0] == 'P') {
      if (request[1] == 'O' && request[2] == 'S' && request[3] == 'T' && request[4] == ' ') {
        position = 5;
        method = POST;
      } else if (request[1] == 'U' && request[2] == 'T' && request[3] == ' ') {
        position = 4;
        method = PUT;
      } else {
        method = ERROR;
        return;
      }
    } else if (request[0] == 'D' && request[1] == 'E' && request[2] == 'L' && request[4] == 'E' &&
               request[5] == 'T' && request[6] == 'E' && request[7] == ' ') {
      method = DELETE;
      return;
    } else {
      method = ERROR;
      return;
    }

    // set the uri to the current position, now that we have the method
    uri = &request[position];


    while (request[position] != '\0' && request[position] != ' ' && request[position] != '\n' && request[position] != '\r') {
      position++;
    }

    // if we're already at the end, we can consider this an invalid request
    if (request[position] == '\0') {
      method = ERROR;

      return;
    }

    // set the uri by terminating the string where we left off
    request[position] = '\0';

    path = uri;

    // and set up the params
    parseParams();

    position++;

    while (request[position] != '\n' && request[position] != '\r' && request[position] != '\0') {
      position++;
    }

    if (request[position] == '\0') {
      return;
    }

    request[position] = '\0';

    position++;

    // move past the LF
    if (request[position + 1] == '\r' || request[position + 1] == '\n') {
      position++;
    }

    position++;

    if (request[position] == '\0') {
      return;
    }

    parseHeaders();

  }

  Method method;
  char *params[MAX_PARAMS] = { 0 };
  char *path = nullptr;
  char *headers[MAX_HEADERS] = { 0 };
  char *body = nullptr;
  char *uri = nullptr;
  char *request = nullptr;

private:
  void parseParams () {
    uint8_t current = 0;
    uint16_t position = 0;

    // figure out where the params start
    while (uri[position] != '\0' && uri[position] != '?') {
      position++;
    }

    if (uri[position] == '?') {
      uri[position] = '\0';
      position++;

      if (uri[position] == '\0') {
        // uri ends with ? and nothing further
        return;
      }
    } else {
      // at the end, no params return NULL
      return;
    }

    // assign the first parameter and move forward
    params[current] = &uri[position];
    current++;

    while (uri[position] != '\0') {
      if (uri[position] == '&') {
        uri[position] = '\0';

        params[current] = &uri[position + 1];
        current++;
        if (uri[position + 1] == '\0') {
          break;
        }
      }

      position++;
    }

    params[current] = nullptr;
  }

  void parseHeaders () {
    uint16_t current = 0;

    if (request[0] == '\0') {
      return;
    }

    uint16_t position = 0;

    headers[current] = request;
    current++;

    while (request[position] != '\0') {
      if (request[position] == '\n' || request[position] == '\r') {
        request[position] = '\0';
        position++;

        if (request[position] == '\r' || request[position] == '\n') {
          if (request[position + 1] == '\r' || request[position + 1] == '\n') {
            break;
          } else {
            position++;
          }
        }

        if (request[position] != '\0') {
          headers[current] = &request[position];
          current++;
        }
      }

      position++;
    }

    headers[current] = NULL;

    if (request[position] != '\0') {
      position++;
    }

    if (request[position] != '\0') {
      position++;
    }

    body = &request[++position];
  }
};

};
