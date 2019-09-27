#include "Rack.hpp"

bool rack_info (HTTP::Request *req, HTTP::Response *res) {
  // rack state
  float sampleRate = APP->engine->getSampleRate();
  bool isPaused = APP->engine->isPaused();

  // quick stupid generation of json
  char buffer[1024];
  sprintf(buffer, "{\"rack\": { \"sampleRate\": %f, \"paused\": %s } }\n", sampleRate, isPaused ? "true" : "false");
  res->setContentType("application/json");
  res->write(buffer);

  return true;
}
