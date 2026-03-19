#pragma once
#include <string>
#include <thread>
#include <memory>
#include <vector>
#include <atomic>

class TCPServer {
private:
    int serverSocket = -1;
    std::atomic<bool> isRunning = false;
    std::unique_ptr<std::thread> serverThread;
    std::vector<int> clientSockets;
    int port;

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

private:
    void acceptConnections();
    void handleClient(int clientSocket);
};
