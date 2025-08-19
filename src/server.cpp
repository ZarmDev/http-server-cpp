#include "lib.h"
#include "utils.h"
#include <cstring>
#include <iostream>
#include <string>

using namespace std;

// Global in this file
string dir;

int main(int argc, char **argv) {
  Server server;

  // https://github.com/varunarya002/codecrafters-http-server-cpp/blob/472d238d47d555645dc8d15081c45fbee8061006/src/server.cpp
  if (argc == 3 && strcmp(argv[1], "--directory") == 0) {
    dir = argv[2];
  }
  cout << "Assuming dir is " << (dir == "" ? "Empty" : dir) << '\n';

  // Initalize server on 4221
  server.init("4221");

  // TODO: routes...

  server.get("/", [](const HttpRequest &req) {
    const string body = "";

    string response = sendString("200 OK", "");
    return response;
  });
  server.get("/useragent", [](const HttpRequest &req) {
    auto userAgent = req.headers.find("User-Agent");
    if (userAgent != req.headers.end())
    {
      string response = sendString("200 OK", userAgent->second);
      return response;
    }
  });
  server.get("/echo", [](const HttpRequest &req) {
    // To prevent substring error (which would crash the server)
    if (req.url.length() > 5)
    {
      const string body = req.url.substr(6);
      string response = sendString("200 OK", body);
      cout << response;
      return response;
    }
    const string response = "HTTP/1.1 404 Not Found\r\n\r\n";
    return response;
  });
  server.get("/files", [](const HttpRequest &req) {
    string fileName = dir + req.url.substr(7);
    cout << "Filename: " << fileName << '\n';
    const string output = readFile(fileName);
    if (output == "")
    {
      string response = sendString("404 Not Found", "");
      return response;
    }
    // Build the response
    Response resp;
    resp.contentType = "application/octet-stream";
    resp.body = output;
    string response = resp.toString();
    cout << response;
    return response;
  });
  server.start();

  return 0;
}