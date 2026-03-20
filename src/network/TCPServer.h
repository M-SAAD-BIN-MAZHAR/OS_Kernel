#pragma once
#include <string>
#include <thread>
#include <memory>
#include <vector>
#include <atomic>
#include <functional>
#include <mutex>

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
    
    // Start the server
    void start();
    
    // Stop the server
    void stop();
    
    // Check if running
    bool running() const { return isRunning; }
    
    // Send message to all connected clients
    void broadcastMessage(const std::string &message);

    // Handle commands received from clients
    void setMessageHandler(std::function<void(const std::string &)> handler);

private:
    void acceptConnections();
    void handleClient(int clientSocket);
};
