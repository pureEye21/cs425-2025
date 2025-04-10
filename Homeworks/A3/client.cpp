#include <iostream>
#include <cstring>           // For memset and other memory functions
#include <sys/socket.h>      // Socket API
#include <arpa/inet.h>       // For inet_addr and related functions
#include <netinet/ip.h>      // IP header structure
#include <netinet/tcp.h>     // TCP header structure
#include <unistd.h>          // For close()
#include <chrono>            // For timeout functionality

// Port definitions
#define SERVER_PORT 12345    // Target server port
#define CLIENT_PORT 80       // Source port for our packets

// Maximum number of retries for sending SYN packet
#define MAX_RETRIES 5

// Utility function to print TCP flags for debugging
void print_tcp_flags(struct tcphdr *tcp) {
    std::cout << "[+] TCP Flags: "
              << " SYN: " << tcp->syn        // Synchronize flag
              << " ACK: " << tcp->ack        // Acknowledgment flag
              << " FIN: " << tcp->fin        // Finish flag
              << " RST: " << tcp->rst        // Reset flag
              << " PSH: " << tcp->psh        // Push flag
              << " SEQ: " << ntohl(tcp->seq) << std::endl; // Sequence number (converted from network to host byte order)
}

/**
 * Sends a SYN packet to initiate TCP handshake
 * @param client_socket The raw socket used for sending
 * @param server_address The target server address
 * @param addr_len Size of the address structure
 * @return 1 on success, 0 on failure
 */
bool send_syn(int client_socket, struct sockaddr_in &server_address, socklen_t &addr_len) {
    // Initialize server address structure
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);   // Convert to network byte order
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");  // Local loopback address

    // Buffer for constructing the packet
    char data_packet[65536];  // Large enough for any IP packet
    memset(data_packet, 0, sizeof(data_packet));  // Initialize to zeros

    // Pointers to IP and TCP headers within our packet buffer
    struct iphdr *ip = (struct iphdr *)data_packet;
    struct tcphdr *tcp = (struct tcphdr *)(data_packet + sizeof(struct iphdr));

    // Fill IP header fields
    ip->ihl = 5;                  // IP header length (5 * 4 = 20 bytes)
    ip->version = 4;              // IPv4
    ip->tos = 0;                  // Type of service
    ip->tot_len = sizeof(struct iphdr) + sizeof(struct tcphdr);  // Total length
    ip->id = htons(54321);        // Identification field
    ip->frag_off = 0;             // No fragmentation
    ip->ttl = 64;                 // Time to live
    ip->protocol = IPPROTO_TCP;   // TCP protocol
    ip->saddr = inet_addr("127.0.0.1");  // Source IP
    ip->daddr = server_address.sin_addr.s_addr;  // Destination IP

    // Fill TCP header fields
    tcp->source = htons(CLIENT_PORT);   // Source port
    tcp->dest = htons(SERVER_PORT);     // Destination port
    tcp->seq = htonl(200);              // Initial sequence number (200 as expected by server)
    tcp->ack_seq = 0;                   // No acknowledgment in SYN packet
    tcp->doff = 5;                      // Data offset (5 * 4 = 20 bytes)
    tcp->ack = 0;                       // ACK flag off
    tcp->syn = 1;                       // SYN flag on - this is a SYN packet
    tcp->window = htons(8192);          // Window size
    tcp->check = 0;                     // Checksum (will be computed by kernel)
        
    // Send the constructed packet
    if (sendto(client_socket, data_packet, ip->tot_len, 0,
            (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("SYN sendto() failed");
        return 0;  // Return failure
    } else {
        std::cout << "[+] SYN packet sent!\n";
        return 1;  // Return success
    }
}

/**
 * Receives and validates SYN-ACK packet from server
 * @param client_socket Socket to receive on
 * @param server_address Server address structure (will be updated)
 * @param addr_len Size of the address structure
 * @return Pointer to TCP header of valid SYN-ACK or nullptr on failure
 */
struct tcphdr* receive_syn_ack(int client_socket, struct sockaddr_in &server_address, socklen_t &addr_len) {
    char buffer[65536];  // Buffer to store received packet
    
    // Receive a packet
    int data_size = recvfrom(client_socket, buffer, sizeof(buffer), 0, 
                            (struct sockaddr *)&server_address, &addr_len);
    if (data_size < 0) {
        return nullptr;  // Error receiving packet
    }

    // Extract IP and TCP headers from the received packet
    struct iphdr *ip = (struct iphdr *)buffer;
    struct tcphdr *tcp = (struct tcphdr *)(buffer + (ip->ihl * 4));  // IP header length might vary

    // Validate source and destination ports
    if ((ntohs(tcp->source) != SERVER_PORT) || (ntohs(tcp->dest) != CLIENT_PORT)) 
        return nullptr;  // Not from our server or not for our client

    // Check if this is a SYN-ACK packet with the expected sequence number
    if (tcp->syn == 1 && tcp->ack == 1 && ntohl(tcp->seq) == 400) {
        std::cout << "[+] Received SYN-ACK from " << inet_ntoa(server_address.sin_addr) << std::endl;
        print_tcp_flags(tcp);
        return tcp;  // Return the TCP header for use in ACK construction
    }

    return nullptr;  // Not the packet we're looking for
}

/**
 * Sends ACK packet to complete the TCP handshake
 * @param client_socket Socket to send on
 * @param server_address Server address structure
 * @param tcp Pointer to the received SYN-ACK TCP header (for sequence numbers)
 * @return 1 on success, 0 on failure
 */
bool send_ack(int client_socket, struct sockaddr_in& server_address, struct tcphdr* tcp) {
    // Set up server address
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Buffer for constructing the packet
    char data_packet[65536];
    memset(data_packet, 0, sizeof(data_packet));

    // Pointers to the different parts of our packet
    struct iphdr *ip = (struct iphdr *)data_packet;
    struct tcphdr *tcp_response = (struct tcphdr *)(data_packet + sizeof(struct iphdr));
    char *data = data_packet + sizeof(struct iphdr) + sizeof(struct tcphdr);

    // Add dummy payload data
    memset(data, 'X', 399);  // 399 bytes of 'X' characters

    // Fill IP header
    ip->ihl = 5;
    ip->version = 4;
    ip->tos = 0;
    ip->tot_len = htons(sizeof(struct iphdr) + sizeof(struct tcphdr) + 399);  // Include payload size
    ip->id = htons(54321);
    ip->frag_off = 0;
    ip->ttl = 64;
    ip->protocol = IPPROTO_TCP;
    ip->saddr = inet_addr("127.0.0.1");
    ip->daddr = server_address.sin_addr.s_addr;

    // Fill TCP header
    tcp_response->source = htons(CLIENT_PORT);
    tcp_response->dest = htons(SERVER_PORT);
    tcp_response->seq = htonl(600);  // Our sequence number for ACK (600 as expected by server)
    tcp_response->ack_seq = htonl(ntohl(tcp->seq) + 1);  // Acknowledge server's sequence number + 1
    tcp_response->doff = 5;
    tcp_response->ack = 1;  // ACK flag on
    tcp_response->syn = 0;  // SYN flag off
    tcp_response->window = htons(8192);
    tcp_response->check = 0;  // Kernel will compute checksum
  
    // Send the ACK packet with payload
    if (sendto(client_socket, data_packet, sizeof(struct iphdr) + sizeof(struct tcphdr) + 399, 0,
            (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("ACK sendto() failed");
        return 0;
    } else {
        std::cout << "[+] ACK packet sent! Handshake Complete\n";
        return 1;
    }
}

int main() {
    int client_socket;

    // Create a raw socket for TCP protocol
    client_socket = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (client_socket < 0) {
        std::cerr << "Error creating socket. \n";
        return 1;
    }

    // Set socket option to include IP header (needed for raw sockets)
    int one = 1;
    if (setsockopt(client_socket, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        perror("setsockopt() failed");
        exit(EXIT_FAILURE);
    }

    // Initialize server address structure
    struct sockaddr_in server_address;
    socklen_t addr_len = sizeof(server_address);
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    int syn_current_count = 0;
    struct tcphdr* tcp;  // Will store received TCP header from SYN-ACK

    // First attempt at sending SYN
    std::cout << "[+] Sending SYN. [Try 1]... \n";
    
    // Retry loop for sending SYN packets
    while(syn_current_count++ < MAX_RETRIES) {
        // Send SYN packet
        if(send_syn(client_socket, server_address, addr_len) == 0) continue;
        std::cout << "[+] Waiting for SYN-ACK... \n";

        bool received = false;
        auto start_time = std::chrono::steady_clock::now();  // Start timeout timer

        // Wait loop for SYN-ACK response with timeout
        while(true) {
            // Calculate elapsed time
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count();
            if (elapsed >= 2) break;  // Exit if 2 seconds passed
            
            // Try to receive SYN-ACK
            tcp = receive_syn_ack(client_socket, server_address, addr_len);
            if (tcp) {
                // SYN-ACK received successfully
                received = true;
                break;
            }
        }

        // Handle timeout situation
        if(!received && syn_current_count < MAX_RETRIES) {
            // Still have retries left
            std::cout << "[!] Timeout waiting for SYN-ACK \n";
            std::cout << "[!] Timeout: Resending SYN [Try " << syn_current_count + 1 << "]... \n";
            continue;
        }
        else if(!received) {
            // Out of retries
            std::cout << "[!] No connection found, Try again \n";
        }
        break;  // Either received or out of retries
    }

    // Only send ACK if we received a valid SYN-ACK
    if(tcp) {
        std::cout << "[+] Sending ACK..." << std::endl;
        while(true) {
            if(send_ack(client_socket, server_address, tcp)) break;
        }
    }

    // Clean up
    close(client_socket);

    return 0;
}