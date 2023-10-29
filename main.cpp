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
#include <fstream>

const int PORT = 8080;
const int MAX_CONNECTIONS = 10;

// Function to read the HTML file and generate the HTML page
std::string generate_html() {
    std::ifstream file("index.html");
    std::string html;
    if (file) {
        std::stringstream buffer;
        buffer << file.rdbuf();
        html = buffer.str();
    } else {
        // Handle the case where the file cannot be opened
        html = "Error: Unable to open the HTML file.";
    }
    return html;
}

// Function to handle client requests
void handle_client(int client_socket) {
    char buffer[1024] = {0};
    int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);

    if (bytes_received < 0) {
        std::cerr << "Error reading from socket" << std::endl;
    } else {
        std::string html_content = generate_html();
        std::string response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Length: " + std::to_string(html_content.length()) + "\r\n";
        response += "Content-Type: text/html\r\n\r\n";
        response += html_content;

        send(client_socket, response.c_str(), response.length(), 0);
    }

    close(client_socket);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        std::cerr << "Error opening socket" << std::endl;
        return 1;
    }

    int reuse = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) < 0) {
        std::cerr << "Error setting socket options" << std::endl;
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Error binding to socket" << std::endl;
        return 1;
    }

    listen(server_socket, MAX_CONNECTIONS);
    std::cout << "Server listening on port " << PORT << std::endl;

    while (true) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            std::cerr << "Error accepting connection" << std::endl;
            return 1;
        }

        std::thread(handle_client, client_socket).detach();
    }

    close(server_socket);

    return 0;
}
