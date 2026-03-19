#include "TCPServer.h"
#include "../utils/Logger.h"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <algorithm>

TCPServer::TCPServer(int port) : port(port) {}

TCPServer::~TCPServer() {
    stop();
}

void TCPServer::start() {
    if (isRunning) return;
    
    isRunning = true;
    serverThread = std::make_unique<std::thread>(&TCPServer::acceptConnections, this);
    Logger::info("TCP Server started on port " + std::to_string(port));
}

void TCPServer::stop() {
    if (!isRunning) return;
    
    isRunning = false;
    
    // Close all client sockets
    for (int clientSocket : clientSockets) {
        close(clientSocket);
    }
    clientSockets.clear();
    
    // Close server socket
    if (serverSocket != -1) {
        close(serverSocket);
        serverSocket = -1;
    }
    
    // Wait for thread to finish
    if (serverThread && serverThread->joinable()) {
        serverThread->join();
    }
    
    Logger::info("TCP Server stopped");
}

void TCPServer::acceptConnections() {
    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        Logger::error("Failed to create socket");
        return;
    }
    
    // Set socket options
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        Logger::error("setsockopt failed");
        close(serverSocket);
        return;
    }
    
    // Bind socket
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);
    
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        Logger::error("Failed to bind socket");
        close(serverSocket);
        return;
    }
    
    // Listen
    listen(serverSocket, 5);
    Logger::info("Server listening on port " + std::to_string(port));
    
    // Accept connections
    while (isRunning) {
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        
        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
        
        if (clientSocket < 0) {
            if (isRunning) {
                Logger::error("Failed to accept connection");
            }
            continue;
        }
        
        Logger::info("New client connected from " + std::string(inet_ntoa(clientAddr.sin_addr)));
        clientSockets.push_back(clientSocket);
    }
}

void TCPServer::broadcastMessage(const std::string &message) {
    std::string msg = message + "\n";
    
    // Remove disconnected clients
    clientSockets.erase(
        std::remove_if(clientSockets.begin(), clientSockets.end(),
            [&msg](int socket) {
                if (send(socket, msg.c_str(), msg.length(), 0) < 0) {
                    close(socket);
                    return true;
                }
                return false;
            }),
        clientSockets.end()
    );
}
