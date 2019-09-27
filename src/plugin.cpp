#include "plugin.hpp"

#include "lib/http/Server.hpp"
#include "lib/routes/FileHandler.hpp"
#include "lib/routes/Rack.hpp"

#include <pthread.h>
#include <regex>

Plugin *pluginInstance;

bool pong(HTTP::Request *req, HTTP::Response *res) {
	res->code = 200;
	res->setContentType("text/html");
	res->write("pong\n");

	return true;
}

void *server_thread (void *arg) {
	HTTP::Server *server = new HTTP::Server(8331);

	server->addRoute(std::regex(".*"), HTTP::GET, fileHandler);
	server->addRoute(std::regex("/ping"), HTTP::GET, pong);
	server->addRoute(std::regex("/api/info"), HTTP::GET, rack_info);

	if (!server->isRunning) {
		rack::WARN("Unable to start HTTP Server: %s", server->error.c_str());
	} else {
		rack::INFO("HTTP Server listening on port %d", 8331);
		server->run();
	}

	return nullptr;
}

void init(Plugin *p) {
	pluginInstance = p;

	pthread_t thread_id;
  pthread_create(&thread_id, NULL, server_thread, NULL);

	// Add modules here
	// p->addModel(modelMyModule);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}
