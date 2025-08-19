// Use header guard to prevent linker issues in server.cpp
#pragma once

using namespace std;

#include <functional>
#include <unordered_map>
#include <string>
#include <optional>
#include <vector>

// Request struct
struct HttpRequest {
  string method;
  string url;
  string protocol;
  unordered_map<string, string> headers;
  string data;
};

// Response function type
using ResponseFunc = const string (*)(const HttpRequest &req);
using RequestFunc = function<string(const HttpRequest &req)>;
using MiddlewareFunc = function<void(const HttpRequest &req)>;

// Server class declaration
class Server {
public:
  bool init(const string &port);
  bool start();
  void get(const string &route, RequestFunc handler);
  void post(const string &route, RequestFunc handler);
  void use(MiddlewareFunc func);
  void use(const std::vector<MiddlewareFunc> &funcs);
  string handleRequest(const HttpRequest &req);
  void setMaxCharacters(int num);

private:
  int server_fd;
  int maximumCharacters = 1024;
  optional<vector<MiddlewareFunc>> currentMiddlewares;
  unordered_map<string, RequestFunc> postRoutes;
  unordered_map<string, RequestFunc> getRoutes;

  void handleClient(int client_fd);
};

// Utility functions
string sendString(const string &status, const string &body);

// Response struct (Help of AI)
struct Response {
  string status = "200 OK";
  string contentType = "text/plain";
  string body = "";

  string toString() const;
};