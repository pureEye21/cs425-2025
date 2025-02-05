# Multi-threaded Chat Server Documentation

## 📋 **Overview**
This document describes a multi-threaded chat server implementation in C++ that enables real-time communication between multiple users. The server supports private messaging, broadcasting, and group chat functionality while maintaining thread safety and concurrent operations. Built with modern C++ features, this server provides a robust foundation for real-time chat applications.

## 🗂️ **Project Structure**
    ├── server_grp.cpp        # Server-side implementation 
    ├── client_grp.cpp        # Client-side implementation 
    ├── users.txt             # User credentials
    └── Makefile              # For compiling the code
---

## 🚀 **Features**
The chat server implementation offers a comprehensive set of communication capabilities designed to support various messaging needs:

- User authentication system that verifies credentials against stored username-password pairs
- Private messaging that enables secure one-to-one communication between users
- Broadcast messaging system allowing users to send messages to all connected clients
- Feature-rich group chat functionality that includes:
  - Group creation with unique naming
  - Dynamic group membership management
  - Targeted group messaging
  - Automatic member cleanup on disconnection
- Multi-user support with concurrent connections through thread-per-client architecture
- Thread-safe operations using mutex locks to prevent data corruption

## ⚙️ **Technical Implementation Details**

### Core Data Structures
The server maintains several interconnected data structures to manage users, connections, and groups effectively:

```cpp
validUsers   # Maps usernames to their corresponding passwords
userSockets  # Maintains active user sessions by mapping usernames to socket descriptors
socketsUser  # Provides reverse lookup from socket descriptors to usernames
groups       # Keep track of all the active clients part of a particular group
```

### Thread Safety Mechanisms
To ensure data consistency in a multi-threaded environment, the server implements two primary synchronization mechanisms:

- `client_mutex`: Guards access to client connection-related data structures
- `group_mutex`: Protects group operations from concurrent modification

## ⚙️ **Compilation Instructions**


### 🚀 **For Linux/WSL:**

1. **Open Terminal and navigate to the project directory.**  
2. Run the following commands:  
   ```
   make all
   ```
## 🏃 Running the Server

After successfully compiling the code, you can run the server and client as follows:

### 🚀 **Step 1: Start the Server**

1. **Open a Terminal (Linux) or Command Prompt (Windows).**  
2. Run the following command:  
    ```
    ./server_grp
    ```
    The server listens on port ```12345``` by default.
### **Server Output Example:**
    Server is listening on port 12345.
    
### 🚀 **Step 2: Client Interaction Example**

1. **Open a Terminal (Linux) or Command Prompt (Windows).**  
2. Run the following command:  
    ```
    ./client_grp
    ```
### **Output Example:**
    Connected to the server.


## 📋 **Usage Guide**

### Available Commands
Users can interact with the server through the following command set:

```
/msg <username> <message>         Send a private message to a specific user
/broadcast <message>              Send a message to all active users
/create_group <groupname>         Create a new chat group
/join_group <groupname>           Become a member of an existing group
/leave_group <groupname>          Exit from a group you've joined
/group_msg <groupname> <message>  Send a message to all group members in a particular group
/exit                             Client disconnects.
```

### Example Usage Scenarios

1. Private Messaging:
```
/msg alice Hey, how are you doing today?
```

2. Broadcasting:
```
/broadcast Important team meeting in 30 minutes!
```

3. Group Operations:
```
/create_group project_team
/join_group project_team
/group_msg project_team Status update: milestone 1 completed
```

## Troubleshooting Guide

### Common Issues and Solutions

1. Server Launch Problems
   - Verify port 12345 availability using netstat
   - Check users.txt file permissions and format
   - Ensure executable permissions are set correctly

2. Authentication Issues
   - Verify users.txt follows the username:password format exactly
   - Remove any trailing whitespace in credentials
   - Confirm case sensitivity matches

3. Connection Difficulties
   - Confirm server process is running
   - Verify correct port configuration (12345)
   - Check network firewall settings

## Security Considerations

While functional for development and testing, users should be aware of these security aspects:

1. Authentication Implementation
   - Current password storage is in plaintext (not recommended for production)
   - Basic authentication mechanism without encryption

2. Network Security
   - Communication is currently unencrypted
   - Susceptible to network packet inspection
   - No protection against man-in-the-middle attacks

3. Input Handling
   - Basic command validation implemented
   - Limited protection against malformed inputs
   - No rate limiting on message sending

## Future Enhancement Possibilities

The server could be enhanced with these improvements:

1. Security Upgrades
   - Implementation of SSL/TLS encryption
   - Secure password hashing using modern algorithms
   - Message rate limiting and flood protection
   - Input sanitization and validation

2. Feature Additions
   - Persistent message history
   - File sharing capabilities
   - User privilege levels and moderation tools
   - Offline message queuing and delivery
   - User presence indicators

3. Performance Optimizations
   - Connection pooling implementation
   - Database integration for persistence
   - Load balancing capabilities
   - Message caching system

## Network Configuration

Default server settings (configurable in source):
```cpp
SERVER_PORT = 12345
MAX_BUFFER_SIZE = 1024
```

These parameters can be adjusted based on deployment requirements and system capabilities.

## Support and Debugging

When reporting issues, provide the following information:
- Complete error messages and stack traces
- Detailed steps to reproduce the issue
- Development environment details:
  - Compiler version and flags
  - Operating system version
  - Relevant system logs
  - Network configuration

---

## 👥 Contributors

| Name            |
|-----------------|
| Rishi Poonia    |
| Dhruv           |
| Yogit           |

---

### 📚 **Course Information**

- **Course:** CS425: Computer Networks  
- **Instructor:** Prof. Adithya Vadapalli
- **Institution:** IIT Kanpur  
