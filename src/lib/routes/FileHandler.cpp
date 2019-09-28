#include <string>
#include <climits>
#include <cstdlib>
#include <sys/stat.h>
#include <stdlib.h>

#include "FileHandler.hpp"

#include "../../plugin.hpp"

static const char *mimeType (std::string suffix) {
  if (suffix == "js") {
    return "application/javascript";
  } else if (suffix == "html") {
    return "text/html";
  } else if (suffix == "css") {
    return "text/css";
  } else if (suffix == "png") {
    return "image/png";
  } else if (suffix == "jpg") {
    return "image/jpeg";
  } else if (suffix == "gif") {
    return "image/gif";
  } else {
    return "text/plain";
  }
}

static char *realPath (char *filepath) {
  std::string base = asset::plugin(pluginInstance, "res/htdocs/");
  std::string fullPath = base + std::string("/") + filepath;
  char editedPath[PATH_MAX];

#ifdef ARCH_WIN
  char *ep = _fullpath(editedPath, base.c_str(), PATH_MAX);
#else
  char *ep = realpath(base.c_str(), editedPath);
#endif

  if (ep == NULL) {
    return nullptr;
  }
  std::string basePath = ep;

  static char actualPath[PATH_MAX];

#ifdef ARCH_WIN
  char *ret = _fullpath(actualPath, fullPath.c_str(), PATH_MAX);
#else
  char *ret = realpath(fullPath.c_str(), actualPath);
#endif

  if (ret) {
    std::string newPath = ret;
    if (newPath.substr(0, basePath.size()) == basePath) {
      return actualPath;
    }
  }

  return nullptr;
}

bool fileHandler (HTTP::Request *req, HTTP::Response *res) {
  char *path = realPath(req->uri);

  if (path) {
    struct stat st;
    size_t size;

    if (stat(path, &st) == 0) {
      size = st.st_size;
    } else {
      return false;
    }

    uint8_t *buffer = new uint8_t[size];

    FILE *in = fopen(path, "rb");
    if (in == NULL) {
      delete[] buffer;
      return false;
    }

    std::string fn = path;
    res->setContentType(mimeType(fn.substr(fn.find_last_of(".") + 1)));
    fread(buffer, sizeof(uint8_t), size, in);
    res->write(buffer, size);
    fclose(in);

    delete[] buffer;

    return true;
  } else {
    return false;
  }
}
