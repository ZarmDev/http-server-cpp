// Use header guard to prevent linker issues in server.cpp
#pragma once

#include <string>
using namespace std;

#pragma once

#include <string>
#include <unordered_map>

// Request struct
struct HttpRequest {
    string method;
    string url;
    string protocol;
    unordered_map<string, string> headers;
    string data;
};

// Response function type
using ResponseFunc = const string (*)(const HttpRequest& req);

// Server class declaration
class Server
{
public:
    bool init();
    bool start(ResponseFunc callback);
private:
    int server_fd;
    static void handleClient(int client_fd, ResponseFunc handleResponse);
};

// Utility functions
string sendString(const string &status, const string &body);

// Response struct (Help of AI)
struct Response
{
    string status = "200 OK";
    string contentType = "text/plain";
    string body = "";

    string toString() const;
};