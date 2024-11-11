#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

// 1. run ip address (result)
// 2. type: "result:4221" in your browser
using namespace std;

int main(int argc, char **argv) {
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;
  
  // You can use print statements as follows for debugging, they'll be visible when running tests.
  std::cout << "Logs from your program will appear here!\n";

  // Uncomment this block to pass the first stage
  // int socket(int domain, int type, int protocol);
  // SOCK_DGRAM - Doesn't require connection to be established, no guarentee for delivery/order/error checking, fixed size. Suitable for zoom meetings, real-time applications, games.
  // SOCK_STREAM - Provides reliable, sequenced packets and error checked packets. It is suitable for applications where data integrity and order are critical, such as HTTP, FTP, and SSH.
  // SOCK_SEQPACKET - Same as SOCK_STREAM but tells you when messages start and end instead of just sending and sending data in one stream. (Maybe?)
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
   std::cerr << "Failed to create server socket\n";
   return 1;
  }

  // Since the tester restarts your program quite often, setting SO_REUSEADDR
  // ensures that we don't run into 'Address already in use' errors
  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
    std::cerr << "setsockopt failed\n";
    return 1;
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(4221);

  if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
    std::cerr << "Failed to bind to port 4221\n";
    return 1;
  }

  int connection_backlog = 5;
  if (listen(server_fd, connection_backlog) != 0) {
    cerr << "listen failed\n";
    return 1;
  }

  struct sockaddr_in client_addr;
  int client_addr_len = sizeof(client_addr);

  std::cout << "Waiting for a client to connect...\n";

  int client_fd = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);
  std::cout << "Client connected\n";

  // Setup a buffer with any amount of characters for now...
  char buffer[4000];
  // https://www.bogotobogo.com/cplusplus/sockets_server_client.php
  // Reads the value from the client and returns the length of the response
  auto n = read(client_fd, buffer,255);
  if (n < 0) cout << ("ERROR reading from socket");
  printf("Here is the message: %s\n",buffer);

  string url = "";
  bool isURL = false;
  for (int i = 0; i < n; i++) {
    // cout << buffer[i];
    if (isURL && buffer[i] == ' ') {
      break;
    } else if (buffer[i] == ' ') {
      isURL = true;
    } else if (isURL == true) {
      url += buffer[i];
    }
  }
  cout << '\n' << url;
  if (url == "/") {
    // cout << "test";
    const char *response = "HTTP/1.1 200 OK\r\n\r\n";

    // send(sockfd, buf, len, flags);
    // buf is the response
    // len is in bytes
    // flags is ???
    send(client_fd, response, strlen(response), 0);
  } else {
      const char *response = "HTTP/1.1 404 Not Found\r\n\r\n";

      // send(sockfd, buf, len, flags);
      // buf is the response
      // len is in bytes
      // flags is ???
      send(client_fd, response, strlen(response), 0);


  }

  close(server_fd);

  return 0;
}
