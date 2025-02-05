#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fstream>

// Port and buffer constants
#define PORT 12345
#define BUFFER_SIZE 1024

// Global containers
std::unordered_map<int, std::string> socketsUser; // maps socket to username
std::unordered_map<std::string, int> userSockets;   // maps username to socket
std::unordered_map<std::string, std::string> validUsers; // valid username:password pairs
std::unordered_map<std::string, std::unordered_set<int>> groups; // group name -> set of sockets

// Global mutexes
std::mutex client_mutex;  // protects socketsUser and userSockets
std::mutex group_mutex;   // protects groups

// Helper function to add a prefix to a message.
std::string add_prefix(std::string sender, std::string message)
{
    return '[' + sender + "]: " + message;
}

// Sends a message to all members of a group except the sender.
void group_message(int sender, std::string group_name, std::string group_msg)
{
    group_msg = add_prefix("Group " + group_name, group_msg);

    // Copy the set of group members under lock.
    std::unordered_set<int> groupClients;
    {
        std::lock_guard<std::mutex> lock(group_mutex);
        // If the group does not exist, simply return.
        if (groups.find(group_name) == groups.end())
            return;
        groupClients = groups[group_name];
    }
    // Send the group message to all members (except the sender)
    for (auto client : groupClients)
    {
        if (client != sender)
        {
            send(client, group_msg.c_str(), group_msg.length(), 0);
        }
    }
}

// Sends a message to all connected clients except the sender.
void broadcast(int sender, std::string message)
{
    // Copy the map of clients under lock.
    std::unordered_map<int, std::string> clients;
    {
        std::lock_guard<std::mutex> lock(client_mutex);
        clients = socketsUser;
    }
    for (auto &[client, user] : clients)
    {
        if (client != sender)
        {
            send(client, message.c_str(), message.length(), 0);
        }
    }
}

// Called when a client disconnects. It removes the client from all the data structures.
void client_disconnected(int socket)
{
    std::string user;
    {
        // Lock both mutexes (order is handled safely with scoped_lock)
        std::scoped_lock lock(client_mutex, group_mutex);
        user = socketsUser[socket];
        // Remove the socket from every group it may be a member of.
        for (auto &[group_name, clients] : groups)
        {
            if (clients.find(socket) != clients.end())
            {
                clients.erase(socket);
            }
        }
        // Remove the client from the client maps.
        socketsUser.erase(socket);
        userSockets.erase(user);
    }
    broadcast(-1, user + " left the chat.");
}

// Simple helper to check if string a starts with string b.
bool starts_with(std::string a, std::string b)
{
    if (a.length() < b.length())
        return false;
    else if (a.substr(0, b.length()) == b)
        return true;
    return false;
}

// This function handles the messages/commands coming from a particular client.
void handle_client_requests(int socket)
{
    const char *noGroupStr = "No such group exists.";
    const char *noUserStr  = "No such user exists.";
    char buffer[BUFFER_SIZE] = {0};
    int bytesReceived;

    while (true)
    {
        bytesReceived = read(socket, buffer, BUFFER_SIZE);
        if (bytesReceived <= 0)
        {
            client_disconnected(socket);
            return;
        }
        else
        {
            std::string message(buffer, bytesReceived);

            if (starts_with(message, "/group_msg"))
            {
                size_t space1 = message.find(' ');
                size_t space2 = message.find(' ', space1 + 1);
                if (space1 != std::string::npos && space2 != std::string::npos)
                {
                    std::string group_name = message.substr(space1 + 1, space2 - space1 - 1);
                    std::string group_msg  = message.substr(space2 + 1);

                    // Check if the group exists under lock.
                    {
                        std::lock_guard<std::mutex> lock(group_mutex);
                        if (groups.find(group_name) == groups.end())
                        {
                            send(socket, noGroupStr, strlen(noGroupStr), 0);
                            continue;
                        }
                    }
                    group_message(socket, group_name, group_msg);
                }
            }
            else if (starts_with(message, "/broadcast"))
            {
                size_t space = message.find(' ');
                if (space != std::string::npos)
                {
                    std::string broadcast_msg = message.substr(space + 1);
                    // Get the sender's name safely.
                    {
                        std::lock_guard<std::mutex> lock(client_mutex);
                        broadcast_msg = add_prefix(socketsUser[socket], broadcast_msg);
                    }
                    broadcast(socket, broadcast_msg);
                }
            }
            else if (starts_with(message, "/msg"))
            {
                size_t space1 = message.find(' ');
                size_t space2 = message.find(' ', space1 + 1);
                if (space1 != std::string::npos && space2 != std::string::npos)
                {
                    std::string receiver = message.substr(space1 + 1, space2 - space1 - 1);
                    std::string msg      = message.substr(space2 + 1);
                    std::string senderName;
                    int receiver_socket;
                    {
                        std::lock_guard<std::mutex> lock(client_mutex);
                        senderName = socketsUser[socket];
                        if (userSockets.find(receiver) == userSockets.end())
                        {
                            send(socket, noUserStr, strlen(noUserStr), 0);
                            continue;
                        }
                        receiver_socket = userSockets[receiver];
                    }
                    msg = add_prefix(senderName, msg);
                    send(receiver_socket, msg.c_str(), msg.length(), 0);
                }
            }
            else if (starts_with(message, "/create_group"))
            {
                size_t space = message.find(' ');
                if (space != std::string::npos)
                {
                    std::string group_name = message.substr(space + 1);
                    std::string groupCreatedStr = "Group " + group_name + " created.";
                    send(socket, groupCreatedStr.c_str(), groupCreatedStr.length(), 0);
                    {
                        std::lock_guard<std::mutex> lock(group_mutex);
                        groups[group_name].insert(socket);
                    }
                }
            }
            else if (starts_with(message, "/join_group"))
            {
                size_t space = message.find(' ');
                if (space != std::string::npos)
                {
                    std::string group_name = message.substr(space + 1);
                    {
                        std::lock_guard<std::mutex> lock(group_mutex);
                        if (groups.find(group_name) == groups.end())
                        {
                            send(socket, noGroupStr, strlen(noGroupStr), 0);
                            continue;
                        }
                        groups[group_name].insert(socket);
                    }
                    std::string joinGroupStr = "You joined the group " + group_name + '.';
                    send(socket, joinGroupStr.c_str(), joinGroupStr.length(), 0);
                }
            }
            else if (starts_with(message, "/leave_group"))
            {
                size_t space = message.find(' ');
                if (space != std::string::npos)
                {
                    std::string group_name = message.substr(space + 1);
                    {
                        std::lock_guard<std::mutex> lock(group_mutex);
                        if (groups.find(group_name) == groups.end())
                        {
                            send(socket, noGroupStr, strlen(noGroupStr), 0);
                            continue;
                        }
                        if (groups[group_name].find(socket) != groups[group_name].end())
                        {
                            groups[group_name].erase(socket);
                        }
                    }
                    std::string groupLeftStr = "You left the group " + group_name + '.';
                    send(socket, groupLeftStr.c_str(), groupLeftStr.length(), 0);
                }
            }
        }
    }
}

// Reads the users.txt file and fills in the validUsers map.
void parseUserstxt()
{
    std::ifstream file("users.txt");
    if (!file)
    {
        std::cerr << "users.txt not found!";
        exit(EXIT_FAILURE);
    }
    std::string userpass;
    while (std::getline(file, userpass))
    {
        int idx = 0, strLen = userpass.length();
        for (; idx < strLen; idx++)
        {
            if (userpass[idx] == ':')
                break;
        }
        validUsers[userpass.substr(0, idx)] = userpass.substr(idx + 1);
    }
}

int main()
{
    int new_socket;
    const char *userStr = "Enter username: ";
    const char *passStr = "Enter password: ";
    const char *welcomeStr = "Welcome to the server";
    const char *authFailedStr = "Authentication Failed";
    char buffer[BUFFER_SIZE] = {0};
    int bytesReceived;
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    parseUserstxt();

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options (macOS compatible)
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket to the address
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0)
    {
        perror("Listen");
        exit(EXIT_FAILURE);
    }

    while (true)
    {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("Accept");
            exit(EXIT_FAILURE);
        }

        send(new_socket, userStr, strlen(userStr), 0);
        bytesReceived = read(new_socket, buffer, BUFFER_SIZE);
        std::string user(buffer, bytesReceived);

        send(new_socket, passStr, strlen(passStr), 0);
        bytesReceived = read(new_socket, buffer, BUFFER_SIZE);
        std::string pass(buffer, bytesReceived);

        if (validUsers.find(user) != validUsers.end() && validUsers[user] == pass)
        {
            std::string newUserStr = user + " has joined the chat.";
            broadcast(-1, newUserStr);

            // Protect client maps while adding a new client.
            {
                std::lock_guard<std::mutex> lock(client_mutex);
                socketsUser[new_socket] = user;
                userSockets[user] = new_socket;
            }

            send(new_socket, welcomeStr, strlen(welcomeStr), 0);
            // Start a thread to handle this client's requests.
            std::thread new_client_thread(handle_client_requests, new_socket);
            new_client_thread.detach();
        }
        else
        {
            send(new_socket, authFailedStr, strlen(authFailedStr), 0);
            close(new_socket);
        }
    }
    close(server_fd);

    return 0;
}
