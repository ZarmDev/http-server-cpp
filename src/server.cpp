// Standard library
#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <unordered_map>
#include <utility>
#include <thread>
// File handling
#include <fstream>
#include <sstream>
// ?
#include <unistd.h>
#include <sys/types.h>
// Socket/linux networking
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

// 1. run ip address (result)
// 2. type: "result:4221" in your browser
using namespace std;
using ResponseFunc = string (*)(const string &, const unordered_map<string, string> &, const string &);

// Global variables
string dir = "";

// Extract the path from a request (example: localhost:4221/test) -> returns test
char *getURLPath(char *buffer, int n)
{
  // Use a static char so it doesn't get deallocated when the function exits (even though the return type is pointer)
  static char url[1024];
  int index = 0;
  bool isURL = false;

  for (int i = 0; i < n; i++)
  {
    if (isURL && buffer[i] == ' ')
    {
      break;
    }
    else if (buffer[i] == ' ')
    {
      isURL = true;
    }
    else if (isURL == true)
    {
      url[index++] = buffer[i];
    }
  }

  url[index] = '\0'; // Null-terminate the string, this signals the end of C style strings
  return url;
}

void fw(string location, string content)
{
  std::ofstream myfile;  // Create an ofstream object
  myfile.open(location); // Open the file

  if (myfile.is_open())
  {                                 // Check if the file is open
    myfile << content << std::endl; // Write to the file
    myfile.close();                 // Close the file
  }
  else
  {
    std::cerr << "Unable to open file"; // Error handling
  }
}

class Server
{
public:
  bool init()
  {
    // Flush after every std::cout / std::cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    // int socket(int domain, int type, int protocol);
    // SOCK_DGRAM - Doesn't require connection to be established, no guarentee for delivery/order/error checking, fixed size. Suitable for zoom meetings, real-time applications, games.
    // SOCK_STREAM - Provides reliable, sequenced packets and error checked packets. It is suitable for applications where data integrity and order are critical, such as HTTP, FTP, and SSH.
    // SOCK_SEQPACKET - Same as SOCK_STREAM but tells you when messages start and end instead of just sending and sending data in one stream. (Maybe?)
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
      std::cerr << "Failed to create server socket\n";
      return false;
    }

    // Since the tester restarts your program quite often, setting SO_REUSEADDR
    // ensures that we don't run into 'Address already in use' errors
    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
    {
      std::cerr << "setsockopt failed\n";
      return false;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(4221);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0)
    {
      std::cerr << "Failed to bind to port 4221\n";
      return false;
    }

    int connection_backlog = 5;
    if (listen(server_fd, connection_backlog) != 0)
    {
      cerr << "listen failed\n";
      return false;
    }

    return true;
  }
  bool start(ResponseFunc callback)
  {
    // This holds the IP address, port number, address type (IPV4/6)
    struct sockaddr_in client_addr;
    // This tells the accept function that the struct is x bytes. It doesn't change on new requests, this is just for the compiler to know
    int client_addr_len = sizeof(client_addr);

    std::cout << "Waiting for a client to connect...\n";

    bool debug = false;

    if (debug)
    {
      int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t *)&client_addr_len);
      std::cout << "Client connected\n";
      // Create a thread, to handle it and then just run it seperately so the program can continue accepting requests
      thread(handleClient, client_fd, callback).detach();
    }
    else
    {
      while (true)
      {
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t *)&client_addr_len);
        std::cout << "Client connected\n";
        // Create a thread, to handle it and then just run it seperately so the program can continue accepting requests
        thread(handleClient, client_fd, callback).detach();
      }
    }
    close(server_fd);
    return true;
  }

private:
  int server_fd;
  // I saw an example on Codecrafters doing it this way and I have to say they did a good job
  static void handleClient(int client_fd, ResponseFunc handleResponse)
  {
    constexpr int maximumCharacters = 1024;
    // Setup a buffer with any amount of characters for now...
    char buffer[maximumCharacters];

    // https://www.bogotobogo.com/cplusplus/sockets_server_client.php
    // Reads the value from the client and returns the length of the response
    int n = read(client_fd, buffer, maximumCharacters);
    if (n < 0)
      cout << ("ERROR reading from socket");
    // printf("Here is the message: %s\n",buffer);
    string bufferStr(buffer);
    // Check if it exceeds the maximum character size (the read function cuts it off anyways so you can just use == to check)
    if (bufferStr.length() == maximumCharacters)
    {
      return;
    }

    // Artificially add \n to ensure that all data is pushed to the map below
    strcat(buffer, "\n");

    string curr;
    string requestType;
    string url;
    string protocol;
    // Unfortunately it says that headers are NOT always in the same order, so we have to use a map for each header
    // According to AI, you should use a unordered map...
    unordered_map<string, string> headers;
    vector<string> lines = {};
    enum STATE
    {
      TYPE,
      URL,
      PROTO,
      FIRST,
      SECOND
    };
    enum STATE state = STATE::TYPE;
    pair<string, string> headerData;
    // Loop through each character in the char* array until the null byte
    for (int i = 0; buffer[i] != '\0'; ++i)
    {
      char c = buffer[i];
      if (state == STATE::TYPE)
      {
        if (c == ' ')
        {
          requestType = curr;
          curr = "";
          state = STATE::URL;
        }
        else
        {
          curr += c;
        }
      }
      else if (state == STATE::URL)
      {
        if (c == ' ')
        {
          url = curr;
          curr = "";
          state = STATE::PROTO;
        }
        else
        {
          curr += c;
        }
      }
      else if (state == STATE::PROTO)
      {
        if (c == ' ')
        {
          protocol = curr;
          curr = "";
          i += 3;
          state = STATE::FIRST;
        }
        else
        {
          curr += c;
        }
      }
      else if (state == STATE::FIRST)
      {
        if (c == ':')
        {
          headerData.first = curr;
          curr = "";
          state = STATE::SECOND;
          i++;
        }
        else
        {
          curr += c;
        }
      }
      else if (state == STATE::SECOND)
      {
        if (c == '\r')
        {
          headerData.second = curr;
          headers.insert(headerData);
          curr = "";
          state = STATE::FIRST;
          i++;
        }
        else
        {
          curr += c;
        }
      }
    }

    // cout << requestType << url << protocol << '\n';
    // for (const auto &pair : headers)
    // {
    //   std::cout << "[" << pair.first << "] = [" << pair.second << "]\n";
    // }

    cout << "INFO: Request to " << url << '\n';

    string response = handleResponse(url, headers, "");

    // send(sockfd, buf, len, flags);
    // buf is the response
    // len is in bytes
    // flags is ???
    send(client_fd, response.c_str(), response.length(), 0);

    fw("output.txt", bufferStr);
  }
};

// Help of AI
struct Response
{
  string status = "200 OK";
  string contentType = "text/plain";
  string body = "";

  string toString() const
  {
    string response = "HTTP/1.1 " + status + "\r\n";
    response += "Content-Type: " + contentType + "\r\n";
    response += "Content-Length: " + to_string(body.length()) + "\r\n\r\n";
    response += body;
    return response;
  }
};

// Just for quick sending
string sendString(const string &status, const string &body)
{
  string response = "HTTP/1.1 " + status + "\r\n";
  response.append("Content-Type: text/plain\r\n");
  response.append("Content-Length: " + to_string(body.length()) + "\r\n" + "\r\n");
  response.append(body + "\r\n");
  return response;
}

// For efficient checking of the first x characters (generated usign AI)
inline bool startsWith(const std::string &str, const std::string &prefix)
{
  return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
}

inline std::string readFile(const std::string &path)
{
  std::ifstream file(path, std::ios::binary);
  if (!file.is_open())
    return "";
  std::ostringstream ss;
  ss << file.rdbuf();
  return ss.str();
}

string handleResponse(const string &url, const unordered_map<string, string> &headers, const string &data)
{
  if (url == "/")
  {
    const string body = "";

    string response = sendString("200 OK", "");
    cout << response;
    return response;
  }
  else if (url == "/user-agent")
  {
    auto userAgent = headers.find("User-Agent");
    if (userAgent != headers.end())
    {
      string response = sendString("200 OK", userAgent->second);

      cout << response;
      return response;
    }
  }
  else if (startsWith(url, "/echo"))
  {
    // To prevent substring error (which would crash the server)
    if (url.length() > 5)
    {
      const string body = url.substr(6);
      string response = sendString("200 OK", body);
      cout << response;
      return response;
    }
    else
    {
      const string response = "HTTP/1.1 404 Not Found\r\n\r\n";
      return response;
    }
  }
  else if (startsWith(url, "/files"))
  {
    string fileName = dir + url.substr(7);
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
  }
  else
  {
    string response = sendString("404 Not Found", "");
    return response;
  }
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
  server.init();
  server.start(handleResponse);
  return 0;
}