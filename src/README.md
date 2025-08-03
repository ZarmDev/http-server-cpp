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
