# Linux socket manual
## Socket() command and uses
TCP (SOCK_STREAM with IPPROTO_TCP):
Reliable Communication: When you need guaranteed delivery, order, and error-checking.

Connection-Oriented: Applications that require a continuous connection, such as:

Web Browsing: HTTP/HTTPS

File Transfer: FTP

Email: SMTP/IMAP/POP3

UDP (SOCK_DGRAM with IPPROTO_UDP):
Speed and Efficiency: When you need low-latency communication and can tolerate occasional loss of packets.

Connectionless: Applications that don't require a continuous connection, such as:

Live Streaming: Video or audio streaming

Online Gaming: Real-time multiplayer games

VoIP: Voice over IP calls

ICMP (IPPROTO_ICMP):
Network Diagnostics: When you need to diagnose network issues, such as:

Ping: Checking the reachability of a host

Traceroute: Determining the path to a host

SCTP (IPPROTO_SCTP):
Multi-Stream Communication: When you need reliable transport with multiple streams and better data delivery mechanisms than TCP. Used in:

Telecommunications: Transporting SS7 signaling messages over IP networks

Raw Sockets:
Custom Protocols: When you need to implement a custom protocol or directly manipulate IP packets. Typically used by:

Network Utilities: Custom network tools and utilities

Security Applications: Custom security scanning tools
## Protocols
1. 0 (Zero):

Default Protocol: Automatically selects the appropriate protocol. For SOCK_STREAM and AF_INET, this would typically mean TCP. For SOCK_DGRAM and AF_INET, it would mean UDP.

2. For AF_INET (IPv4):

IPPROTO_TCP: Value 6, used with SOCK_STREAM for TCP.

int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
IPPROTO_UDP: Value 17, used with SOCK_DGRAM for UDP.

int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
3. For AF_INET6 (IPv6):

IPPROTO_TCP: Same value 6, used for TCP over IPv6.

IPPROTO_UDP: Same value 17, used for UDP over IPv6.

4. Other Protocols:

There are other protocols like IPPROTO_ICMP for ICMP, but these are less common for typical socket programming tasks.

## Ideas for HTTPS
https://codereview.stackexchange.com/questions/264375/a-c-library-that-sends-and-receives-data-using-https-or-http

Or check source code for https://github.com/yhirose/cpp-httplib

Copilot idea:
# Adding HTTPS Support to Your HTTP Server

Adding HTTPS to your server requires integrating TLS/SSL capabilities. Here's a general approach:

## 1. Add OpenSSL library

```sh
# Install the development libraries
sudo apt-get install libssl-dev
```

## 2. Update your build system

Add to your CMakeLists.txt:
```cmake
find_package(OpenSSL REQUIRED)
target_link_libraries(server OpenSSL::SSL OpenSSL::Crypto)
```

## 3. Create/obtain SSL certificates

For testing:
```sh
openssl req -x509 -newkey rsa:4096 -keyout key.pem -out cert.pem -days 365 -nodes
```

## 4. Modify your server code

```cpp
#include <openssl/ssl.h>
#include <openssl/err.h>

class SecureServer : public Server {
private:
    SSL_CTX *ssl_ctx;
    
public:
    bool init_ssl() {
        // Initialize OpenSSL
        SSL_load_error_strings();
        OpenSSL_add_ssl_algorithms();
        
        // Create SSL context
        const SSL_METHOD *method = TLS_server_method();
        ssl_ctx = SSL_CTX_new(method);
        if (!ssl_ctx) {
            std::cerr << "Failed to create SSL context\n";
            return false;
        }
        
        // Configure certificates
        if (SSL_CTX_use_certificate_file(ssl_ctx, "cert.pem", SSL_FILETYPE_PEM) <= 0) {
            std::cerr << "Failed to load certificate\n";
            return false;
        }
        
        if (SSL_CTX_use_PrivateKey_file(ssl_ctx, "key.pem", SSL_FILETYPE_PEM) <= 0) {
            std::cerr << "Failed to load private key\n";
            return false;
        }
        
        return true;
    }
    
    void handle_secure_client(int client_fd, ResponseFunc handleResponse) {
        // Create SSL structure
        SSL *ssl = SSL_new(ssl_ctx);
        SSL_set_fd(ssl, client_fd);
        
        // Perform TLS handshake
        if (SSL_accept(ssl) <= 0) {
            std::cerr << "SSL handshake failed\n";
            SSL_free(ssl);
            return;
        }
        
        // Read/write using SSL_read/SSL_write instead of read/write
        // ...
        
        // Clean up
        SSL_shutdown(ssl);
        SSL_free(ssl);
    }
};
```

## 5. Update your client handling

Replace the raw socket reads/writes with SSL equivalents:

```cpp
// Instead of:
int n = read(client_fd, buffer, maximumCharacters);
// Use:
int n = SSL_read(ssl, buffer, maximumCharacters);

// Instead of:
send(client_fd, response.c_str(), response.length(), 0);
// Use:
SSL_write(ssl, response.c_str(), response.length());
```

## Key points:

1. You'll need to handle HTTPS on a different port (typically 443)
2. Consider supporting both HTTP and HTTPS simultaneously
3. For production, obtain a real certificate from a Certificate Authority
4. Be mindful of performance - TLS handshakes are computationally expensive

This is a simplified overview. A complete implementation would need error handling, proper certificate verification, and security best practices.
