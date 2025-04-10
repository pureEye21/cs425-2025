# Raw TCP Handshake Implementation

This project demonstrates a manual implementation of the TCP three-way handshake using raw sockets in C++. The implementation consists of two programs:

1. `server.cpp` - Listens for incoming SYN packets, responds with SYN-ACK, and processes ACK packets
2. `client.cpp` - Initiates a TCP handshake by sending SYN, waiting for SYN-ACK, and sending ACK

## Team Members

- Dhruv - 210338
- Rishi Pooni - 210851
- Yogit - 211207

## Overview

The TCP three-way handshake is implemented as follows:

1. **Client → Server**: SYN packet with sequence number 200
2. **Server → Client**: SYN-ACK packet with sequence number 400 and acknowledgment of client's sequence number
3. **Client → Server**: ACK packet with sequence number 600 and acknowledgment of server's sequence number

Both programs construct raw IP and TCP packets manually, providing a detailed view of the TCP handshake process.

## Prerequisites

- Linux operating system (tested on Ubuntu/Debian)
- C++ compiler (g++ recommended)
- Root/sudo privileges (required for raw socket operations)
- Basic network knowledge

## Compilation

Compile both programs using g++:

```bash
g++ -o server server.cpp
g++ -o client client.cpp
```

## Running the Programs

**Important**: Both programs require root privileges to create raw sockets.

1. First, start the server:
```bash
sudo ./server
```

2. Then, in a separate terminal, start the client:
```bash
sudo ./client
```

## Expected Output

### Server Output
```
[+] Server listening on port 12345...
[+] TCP Flags:  SYN: 1 ACK: 0 FIN: 0 RST: 0 PSH: 0 SEQ: 200
[+] Received SYN from 127.0.0.1
[+] Sent SYN-ACK
[+] TCP Flags:  SYN: 0 ACK: 1 FIN: 0 RST: 0 PSH: 0 SEQ: 600
[+] Received ACK, handshake complete.
```

### Client Output
```
[+] Sending SYN. [Try 1]... 
[+] SYN packet sent!
[+] Waiting for SYN-ACK... 
[+] Received SYN-ACK from 127.0.0.1
[+] TCP Flags:  SYN: 1 ACK: 1 FIN: 0 RST: 0 PSH: 0 SEQ: 400
[+] Sending ACK...
[+] ACK packet sent! Handshake Complete
```

## Troubleshooting

1. **Permission Denied**: Make sure to run both programs with sudo or as root.
2. **No Response**: Check if any firewall is blocking the packets.
3. **Timeout**: The client has a retry mechanism. Verify that both programs are running and the ports match.

## Client.cpp Explanation

The client implementation has several key components:

1. **Functions**:
   - `print_tcp_flags()`: Displays the TCP flags and sequence number for debugging
   - `send_syn()`: Creates and sends a SYN packet with sequence number 200
   - `receive_syn_ack()`: Waits for and validates the SYN-ACK response from server
   - `send_ack()`: Sends the final ACK packet with sequence number 600 and payload

2. **Reliability Features**:
   - **MAX_RETRIES (5)**: The client will attempt to send a SYN packet up to 5 times if no response is received
   - **Timeout**: A 2-second timeout is implemented when waiting for SYN-ACK responses
   - **Error Handling**: Proper error checking at each step of the process

3. **Raw Socket Usage**:
   - Creates a raw socket with `SOCK_RAW` and `IPPROTO_TCP` (Port 80)
   - Sets `IP_HDRINCL` option to manually construct IP headers
   - Manually fills in all fields of IP and TCP headers

4. **Handshake Flow**:
   - The main function coordinates the entire process
   - It first sends SYN and waits for SYN-ACK with timeout
   - If SYN-ACK is received, it sends the final ACK with payload
   - It includes proper cleanup by closing the socket

## Server.cpp Explanation

The server implementation:

1. **Functions**:
   - `print_tcp_flags()`: Displays TCP flags for debugging
   - `send_syn_ack()`: Constructs and sends SYN-ACK in response to SYN
   - `receive_syn()`: Main listening loop that handles incoming packets

2. **Packet Processing**:
   - Listens for packets destined to SERVER_PORT (12345)
   - Validates SYN packets with sequence number 200
   - Responds with SYN-ACK with sequence number 400
   - Verifies ACK with sequence number 600

## Implementation Details

1. **Raw Sockets**: Both programs use raw sockets to have complete control over the IP and TCP headers.
2. **Manual Packet Construction**: IP and TCP headers are manually constructed.
3. **Sequence Numbers**: Specific sequence numbers (200, 400, 600) are used to validate the correct packet exchange.
4. **Retry Mechanism**: The client implements a timeout and retry system for reliability.
5. **Port Numbers**: Using SERVER_PORT (12345) and CLIENT_PORT (80) for communication.

## Notes

- This implementation is for educational purposes and demonstrates the concepts of the TCP handshake.
- The code uses fixed sequence numbers and hardcoded IP addresses for simplicity.
- In real-world TCP implementations, sequence numbers would be randomly generated for security.
- The implementation does not handle all edge cases that a production TCP stack would handle.

## Academic Information

- Course: Computer Networks
- Instructor: Adithya Vadapalli
- TAs: Mohan, Viren, Prakhar
