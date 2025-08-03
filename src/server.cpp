#include "lib.h"
#include "utils.h"
#include <iostream>
#include <string>
#include <cstring>

using namespace std;

// Global variables
string dir = "";

const string handleGETResponse(const HttpRequest &req)
{
  if (req.url == "/")
  {
    const string body = "";

    const string response = sendString("200 OK", "");
    return response;
  }
  else if (req.url == "/user-agent")
  {
    auto userAgent = req.headers.find("User-Agent");
    if (userAgent != req.headers.end())
    {
      const string response = sendString("200 OK", userAgent->second);
      return response;
    }
  }
  else if (startsWith(req.url, "/echo"))
  {
    // To prevent substring error (which would crash the server)
    if (req.url.length() > 5)
    {
      const string body = req.url.substr(6);
      const string response = sendString("200 OK", body);
      return response;
    }
    else
    {
      const string response = "HTTP/1.1 404 Not Found\r\n\r\n";
      return response;
    }
  }
  else if (startsWith(req.url, "/files"))
  {
    string fileName = dir + req.url.substr(7);
    cout << "Filename: " << fileName << '\n';
    const string output = readFile(fileName);
    if (output == "")
    {
      const string response = sendString("404 Not Found", "");
      return response;
    }
    // Build the response
    Response resp;
    resp.contentType = "application/octet-stream";
    resp.body = output;
    string response = resp.toString();
    return response;
  }
  // Else condition
  return sendString("404 Not Found", "");
}

const string handlePOSTResponse(const HttpRequest &req)
{
  if (startsWith(req.url, "/files"))
  {
    string fileName = dir + req.url.substr(7);
    cout << "Filename: " << fileName << '\n';
    if (fileName == "")
    {
      const string response = sendString("404 Not Found", "");
      return response;
    }
    cout << "Data: " << req.data << '\n';
    writeToFile(fileName, req.data);
    // Build the response
    const string response = sendString("201 Created", "");
    return response;
  }
  // Else condition
  return sendString("404 Not Found", "");
}

const string middleware(const HttpRequest& req)
{
  if (req.method == "GET")
  {
    return handleGETResponse(req);
  }
  else if (req.method == "POST")
  {
    return handlePOSTResponse(req);
  }
  // Else condition
  const string response = sendString("404 Not Found", "");
  return response;
}

int main(int argc, char **argv)
{
  Server server;
  // server.setMaxCharLength(int);
  // https://github.com/varunarya002/codecrafters-http-server-cpp/blob/472d238d47d555645dc8d15081c45fbee8061006/src/server.cpp
  if (argc == 3 && strcmp(argv[1], "--directory") == 0)
  {
    dir = argv[2];
  }
  cout << "Assuming dir is " << (dir == "" ? "Empty" : dir) << '\n';
  server.init();
  server.start(middleware);
  return 0;
}