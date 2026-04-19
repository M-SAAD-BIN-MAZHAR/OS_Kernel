#include "TCPServer.h"
#include "../utils/Logger.h"
#include <algorithm>
#include <arpa/inet.h>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

TCPServer::TCPServer(int port) : port(port) {}

TCPServer::~TCPServer() {
    stop();
}

void TCPServer::start() {
    if (isRunning) {
        return;
    }

    isRunning = true;
    serverThread = std::make_unique<std::thread>(&TCPServer::acceptConnections, this);
    Logger::info("TCP Server started on port " + std::to_string(port));
}

void TCPServer::stop() {
    if (!isRunning) {
        return;
    }

    isRunning = false;
    {
        std::lock_guard<std::mutex> lock(clientMutex);
        for (int clientSocket : clientSockets) {
            close(clientSocket);
        }
        clientSockets.clear();
    }

    if (serverSocket != -1) {
        close(serverSocket);
        serverSocket = -1;
    }

    if (serverThread && serverThread->joinable()) {
        serverThread->join();
    }

    Logger::info("TCP Server stopped");
}

void TCPServer::acceptConnections() {
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        Logger::error("Failed to create socket");
        return;
    }

    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        Logger::error("setsockopt failed");
        close(serverSocket);
        return;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, reinterpret_cast<sockaddr *>(&serverAddr), sizeof(serverAddr)) < 0) {
        Logger::error("Failed to bind socket");
        close(serverSocket);
        return;
    }

    listen(serverSocket, 5);
    Logger::info("Server listening on port " + std::to_string(port));

    while (isRunning) {
        sockaddr_in clientAddr{};
        socklen_t clientAddrLen = sizeof(clientAddr);
        const int clientSocket = accept(serverSocket, reinterpret_cast<sockaddr *>(&clientAddr), &clientAddrLen);

        if (clientSocket < 0) {
            if (isRunning) {
                Logger::error("Failed to accept connection");
            }
            continue;
        }

        Logger::info("New client connected from " + std::string(inet_ntoa(clientAddr.sin_addr)));
        {
            std::lock_guard<std::mutex> lock(clientMutex);
            clientSockets.push_back(clientSocket);
        }

        std::thread clientThread(&TCPServer::handleClient, this, clientSocket);
        clientThread.detach();
    }
}

void TCPServer::broadcastMessage(const std::string &message) {
    const std::string msg = message + "\n";

    std::lock_guard<std::mutex> lock(clientMutex);
    clientSockets.erase(std::remove_if(clientSockets.begin(), clientSockets.end(), [&msg](int socket) {
                           if (send(socket, msg.c_str(), msg.length(), 0) < 0) {
                               close(socket);
                               return true;
                           }
                           return false;
                       }),
                       clientSockets.end());
}

void TCPServer::setMessageHandler(std::function<void(const std::string &)> handler) {
    messageHandler = std::move(handler);
}

void TCPServer::handleClient(int clientSocket) {
    constexpr int kBufferSize = 1024;
    char buffer[kBufferSize];
    std::string pending;

    while (isRunning) {
        const int bytesRead = recv(clientSocket, buffer, kBufferSize - 1, 0);
        if (bytesRead <= 0) {
            break;
        }

        buffer[bytesRead] = '\0';
        pending.append(buffer);

        std::size_t newlinePos = std::string::npos;
        while ((newlinePos = pending.find('\n')) != std::string::npos) {
            std::string message = pending.substr(0, newlinePos);
            pending.erase(0, newlinePos + 1);
            if (!message.empty() && messageHandler) {
                messageHandler(message);
            }
        }
    }

    close(clientSocket);
    std::lock_guard<std::mutex> lock(clientMutex);
    clientSockets.erase(std::remove(clientSockets.begin(), clientSockets.end(), clientSocket), clientSockets.end());
}
