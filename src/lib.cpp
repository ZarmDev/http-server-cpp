
// Standard library
#include <cstring>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>
// ?
#include <sys/types.h>
#include <unistd.h>
// Socket/linux networking
#include "lib.h"
#include "utils.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>

using namespace std;

// using ResponseFunc = const string (*)(const HttpRequest &req);
// using RequestFunc = function<void(const HttpRequest &req)>;

// Extract the path from a request (example: localhost:4221/test) -> returns
// test
char *getURLPath(char *buffer, int n) {
  // Use a static char so it doesn't get deallocated when the function exits
  // (even though the return type is pointer)
  static char url[1024];
  int index = 0;
  bool isURL = false;

  for (int i = 0; i < n; i++) {
    if (isURL && buffer[i] == ' ') {
      break;
    } else if (buffer[i] == ' ') {
      isURL = true;
    } else if (isURL == true) {
      url[index++] = buffer[i];
    }
  }

  url[index] = '\0'; // Null-terminate the string, this signals the end of C
                     // style strings
  return url;
}
bool Server::init(const string &port) {
  // Flush after every cout / cerr
  cout << unitbuf;
  cerr << unitbuf;

  // int socket(int domain, int type, int protocol);
  // SOCK_DGRAM - Doesn't require connection to be established, no guarentee for
  // delivery/order/error checking, fixed size. Suitable for zoom meetings,
  // real-time applications, games. SOCK_STREAM - Provides reliable, sequenced
  // packets and error checked packets. It is suitable for applications where
  // data integrity and order are critical, such as HTTP, FTP, and SSH.
  // SOCK_SEQPACKET - Same as SOCK_STREAM but tells you when messages start and
  // end instead of just sending and sending data in one stream. (Maybe?)
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    cerr << "Failed to create server socket\n";
    return false;
  }

  // Since the tester restarts your program quite often, setting SO_REUSEADDR
  // ensures that we don't run into 'Address already in use' errors
  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) <
      0) {
    cerr << "setsockopt failed\n";
    return false;
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(4221);

  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) !=
      0) {
    cerr << "Failed to bind to port " + port + "\n";
    return false;
  }

  int connection_backlog = 5;
  if (listen(server_fd, connection_backlog) != 0) {
    cerr << "listen failed\n";
    return false;
  }

  cout << "Binded server on port: " + port + "\n";

  return true;
}
bool Server::start() {
  // This holds the IP address, port number, address type (IPV4/6)
  struct sockaddr_in client_addr;
  // This tells the accept function that the struct is x bytes. It doesn't
  // change on new requests, this is just for the compiler to know
  int client_addr_len = sizeof(client_addr);

  while (true) {
    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr,
                           (socklen_t *)&client_addr_len);
    cout << "Client connected\n";
    try {
      // Create a thread, to handle it and then just run it seperately so the
      // program can continue accepting requests
      // Non static functions need to accept this when creating threads
      thread(&Server::handleClient, this, client_fd).detach();
    } catch (int err) {
      cout << "An error occured during processing. Error code: " << err << "\n";
    }
  }
  close(server_fd);
  return true;
}

string replace_all(string str, const string &from, const string &to) {
  size_t start_pos = 0; // Initialize starting position for search
  // Loop as long as the 'from' substring is found
  while ((start_pos = str.find(from, start_pos)) != string::npos) {
    // Replace the found occurrence
    str.replace(start_pos, from.length(), to);
    // Advance the starting position to avoid replacing the newly inserted 'to'
    // string if it contains the 'from' substring itself (e.g., replacing "x"
    // with "yx")
    start_pos += to.length();
  }
  return str; // Return the modified string
}

// I saw an example on Codecrafters doing it this way and I have to say they did
// a good job
void Server::handleClient(int client_fd) {
  // Setup a buffer with any amount of characters for now...
  char buffer[maximumCharacters];

  // https://www.bogotobogo.com/cplusplus/sockets_server_client.php
  // Reads the value from the client and returns the length of the response
  int n = read(client_fd, buffer, maximumCharacters);
  if (n < 0)
    cout << ("ERROR reading from socket");
  // printf("Here is the message: %s\n",buffer);
  string bufferStr(buffer);
  // Check if it exceeds the maximum character size (the read function cuts it
  // off anyways so you can just use == to check)
  if (bufferStr.length() == maximumCharacters) {
    return;
  }

  // Artificially add \n to ensure that all data is pushed to the map below
  strcat(buffer, "\n");

  string curr;
  string httpMethod;
  string url;
  string protocol;
  string data;
  // Unfortunately it says that headers are NOT always in the same order, so we
  // have to use a map for each header According to AI, you should use a
  // unordered map...
  unordered_map<string, string> headers;
  vector<string> lines = {};
  enum STATE { TYPE, URL, PROTO, FIRST, SECOND, DATA };
  enum STATE state = STATE::TYPE;
  pair<string, string> headerData;
  // Loop through each character in the char* array until the null byte
  for (int i = 0; buffer[i] != '\0'; ++i) {
    char c = buffer[i];
    if (state == STATE::TYPE) {
      if (c == ' ') {
        httpMethod = curr;
        curr = "";
        state = STATE::URL;
      } else {
        curr += c;
      }
    } else if (state == STATE::URL) {
      if (c == ' ') {
        url = curr;
        curr = "";
        state = STATE::PROTO;
      } else {
        curr += c;
      }
    } else if (state == STATE::PROTO) {
      if (c == '\r') {
        protocol = curr;
        curr = "";
        i++;
        state = STATE::FIRST;
      } else {
        curr += c;
      }
    } else if (state == STATE::FIRST) {
      if (c == '\r') {
        curr = "";
        state = STATE::DATA;
        i++;
      } else if (c == ':') {
        headerData.first = curr;
        curr = "";
        state = STATE::SECOND;
        i++;
      } else {
        curr += c;
      }
    } else if (state == STATE::SECOND) {
      if (c == '\r') {
        headerData.second = curr;
        headers.insert(headerData);
        curr = "";
        state = STATE::FIRST;
        i++;
      } else {
        curr += c;
      }
    } else if (state == STATE::DATA) {
      // No need to check anything else since this will terminate after the
      // request body is over
      curr += c;
    }
  }
  data = curr;
  // Since there will be a \n at the end of checking (O(1) time efficiency)
  if (!data.empty()) {
    data.pop_back();
  }

  writeToFile("output.txt", bufferStr);

  cout << "INFO: Request to " << url << '\n';
  cout << httpMethod << "\n" << protocol << '\n';
  for (const auto &pair : headers) {
    cout << "[" << pair.first << "] = [" << pair.second << "]\n";
  }

  HttpRequest req = {httpMethod, url, protocol, headers, data};
  string response = this->handleRequest(req);

  // send(sockfd, buf, len, flags);
  // buf is the response
  // len is in bytes
  // flags is ???
  send(client_fd, response.c_str(), response.length(), 0);
}

void Server::setMaxCharacters(int num) {
  maximumCharacters = num;
}

void Server::use(MiddlewareFunc func) {
  currentMiddlewares = std::vector<MiddlewareFunc>{func};
}

void Server::use(const vector<MiddlewareFunc> &funcs) {
  currentMiddlewares = funcs;
}

void Server::get(const string &route, RequestFunc handler) {
  getRoutes[route] = handler;
}

void Server::post(const string &route, RequestFunc handler) {
  postRoutes[route] = handler;
}

string Server::handleRequest(const HttpRequest &req) {
  // First, handle middleware
  if (currentMiddlewares.has_value()) {
    for (const auto &mw : currentMiddlewares.value()) {
      mw(req); // Call the middleware function
    }
  }
  if (req.method == "GET") {
    // This just finds the requested URL in the map with routes
    auto it = getRoutes.find(req.url);
    if (it != getRoutes.end()) {
      // Access the function using it->second and call it with the request
      return it->second(req);
    } else {
      return sendString("404 Not Found", "");
    }
  } else if (req.method == "POST") {
    // This just finds the requested URL in the map with routes
    auto it = postRoutes.find(req.url);
    if (it != postRoutes.end()) {
      // Access the function using it->second and call it with the request
      return it->second(req);
    } else {
      return sendString("404 Not Found", "");
    }
  }
  return sendString("404 Not Found", "");
}

// TODO: parse multipart/form-data
/*
POST /upload HTTP/1.1
Content-Type: multipart/form-data; boundary=----WebKitFormBoundaryX

------WebKitFormBoundaryX
Content-Disposition: form-data; name="file"; filename="myfile.pdf"
Content-Type: application/pdf

(binary PDF data here)
------WebKitFormBoundaryX--
*/

// Help of AI
string Response::toString() const {
  string response = "HTTP/1.1 " + status + "\r\n";
  response += "Content-Type: " + contentType + "\r\n";
  response += "Content-Length: " + to_string(body.length()) + "\r\n\r\n";
  response += body;
  return response;
}

// Just for quick sending
string sendString(const string &status, const string &body) {
  string response = "HTTP/1.1 " + status + "\r\n";
  response.append("Content-Type: text/plain\r\n");
  response.append("Content-Length: " + to_string(body.length()) + "\r\n" +
                  "\r\n");
  response.append(body + "\r\n");
  return response;
}