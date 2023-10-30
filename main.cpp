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
    char buffer[4096] = {0};  // Increased buffer size for better handling of HTTP headers.
    int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);

    if (bytes_received < 0) {
        std::cerr << "Error reading from socket" << std::endl;
    } else {
        std::string request(buffer);

        if (request.find("GET / ") != std::string::npos) {
            std::string html_response = "HTTP/1.1 200 OK\r\n";
            html_response += "Content-Type: text/html\r\n\r\n";
            html_response += generate_html();
            send(client_socket, html_response.c_str(), html_response.length(), 0);
        } else if (request.find("POST /calculate") != std::string::npos) {
            std::string html_response = "HTTP/1.1 200 OK\r\n";
            html_response += "Content-Type: text/plain\r\n\r\n";

            size_t pos = request.find("num1=");
            if (pos != std::string::npos) {
                std::string num1_str = request.substr(pos + 5);
                pos = num1_str.find("&");
                if (pos != std::string::npos) {
                    num1_str = num1_str.substr(0, pos);
                }

                pos = request.find("num2=");
                if (pos != std::string::npos) {
                    std::string num2_str = request.substr(pos + 5);
                    int num1, num2;

                    try {
                        num1 = std::stoi(num1_str);
                        num2 = std::stoi(num2_str);

                        int sum = num1 + num2;

                        html_response += std::to_string(sum);
                    } catch (const std::invalid_argument&) {
                        html_response += "Invalid input: Both num1 and num2 must be valid integers.";
                    }
                }
            }

            send(client_socket, html_response.c_str(), html_response.length(), 0);
        }
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
