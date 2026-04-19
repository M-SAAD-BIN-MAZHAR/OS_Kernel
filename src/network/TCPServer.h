#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

class TCPServer {
private:
    int serverSocket = -1;
    std::atomic<bool> isRunning = false;
    std::unique_ptr<std::thread> serverThread;
    std::vector<int> clientSockets;
    std::mutex clientMutex;
    int port;
    std::function<void(const std::string &)> messageHandler;

public:
    explicit TCPServer(int port = 9000);
    ~TCPServer();

    void start();
    void stop();
    bool running() const { return isRunning; }
    void broadcastMessage(const std::string &message);
    void setMessageHandler(std::function<void(const std::string &)> handler);

private:
    void acceptConnections();
    void handleClient(int clientSocket);
};
