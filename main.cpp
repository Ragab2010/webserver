#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

// Constants
const int PORT = 8080;
const int MAX_CONNECTIONS = 10;

// HTTP response
const std::string RESPONSE = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";

// Function to get the current time
std::string get_current_time() {
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    return std::ctime(&now_c);
}

// Function to handle an incoming client connection
void handle_client(int client_socket) {
    char buffer[1024] = {0};
    int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);

    if (bytes_received < 0) {
        std::cerr << "Error reading from socket" << std::endl;
    } else {
        // Respond to the client
        send(client_socket, RESPONSE.c_str(), RESPONSE.length(), 0);
    }

    // Close the client socket
    close(client_socket);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // Create the server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        std::cerr << "Error opening socket" << std::endl;
        return 1;
    }

    // Bind the server socket to a specific address and port
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr) ) < 0) {
        std::cerr << "Error binding to socket" << std::endl;
        return 1;
    }

    // Listen for incoming connections
    listen(server_socket, MAX_CONNECTIONS);
    std::cout << "Server listening on port " << PORT << std::endl;

    while (true) {
        // Accept incoming client connections
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            std::cerr << "Error accepting connection" << std::endl;
            return 1;
        }

        // Handle the connection in a separate thread
        std::thread(handle_client, client_socket).detach();
    }

    // Close the server socket
    close(server_socket);

    return 0;
}
