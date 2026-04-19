#include "simulator_client.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>

SimulatorClient::SimulatorClient(const QString &host, quint16 port, QObject *parent)
    : QObject(parent), socket(std::make_unique<QTcpSocket>()), host(host), port(port)
{
    connect(socket.get(), &QTcpSocket::connected, this, &SimulatorClient::onConnected);
    connect(socket.get(), &QTcpSocket::disconnected, this, &SimulatorClient::onDisconnected);
    connect(socket.get(), &QTcpSocket::readyRead, this, &SimulatorClient::onReadyRead);
    connect(socket.get(), &QTcpSocket::errorOccurred, this, &SimulatorClient::onError);
}

SimulatorClient::~SimulatorClient() {
    disconnectFromServer();
}

void SimulatorClient::connectToServer() {
    if (!socket->isOpen()) {
        std::cout << "Connecting to " << host.toStdString() << ":" << port << std::endl;
        socket->connectToHost(host, port);
    }
}

void SimulatorClient::disconnectFromServer() {
    if (socket->isOpen()) {
        socket->disconnectFromHost();
    }
}

bool SimulatorClient::isConnected() const {
    return socket->state() == QTcpSocket::ConnectedState;
}

void SimulatorClient::selectScheduler(const QString &algorithm) {
    QJsonObject obj;
    obj["command"] = "select_scheduler";
    obj["algorithm"] = algorithm;
    sendCommand(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void SimulatorClient::startSimulation() {
    QJsonObject obj;
    obj["command"] = "start_simulation";
    sendCommand(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void SimulatorClient::startSyncDemo(const QString &demo) {
    QJsonObject obj;
    obj["command"] = "start_sync_demo";
    obj["demo"] = demo;
    sendCommand(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void SimulatorClient::startDeadlockDemo(const QString &scenario) {
    QJsonObject obj;
    obj["command"] = "start_deadlock_demo";
    obj["scenario"] = scenario;
    sendCommand(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void SimulatorClient::stopDeadlockDemo() {
    QJsonObject obj;
    obj["command"] = "stop_deadlock_demo";
    sendCommand(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void SimulatorClient::selectMemoryPolicy(const QString &policy) {
    QJsonObject obj;
    obj["command"] = "select_memory_policy";
    obj["policy"] = policy;
    sendCommand(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void SimulatorClient::configureMemory(int pageSize, int frameCount, int tlbSize) {
    QJsonObject obj;
    obj["command"] = "configure_memory";
    obj["pageSize"] = pageSize;
    obj["frameCount"] = frameCount;
    obj["tlbSize"] = tlbSize;
    sendCommand(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void SimulatorClient::startMemorySimulation() {
    QJsonObject obj;
    obj["command"] = "start_memory_simulation";
    sendCommand(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void SimulatorClient::sendCommand(const QString &command) {
    if (socket && socket->isOpen()) {
        socket->write((command + "\n").toUtf8());
        socket->flush();
    }
}

void SimulatorClient::onConnected() {
    std::cout << "Connected to server!" << std::endl;
    emit connected();
}

void SimulatorClient::onDisconnected() {
    std::cout << "Disconnected from server" << std::endl;
    emit disconnected();
}

void SimulatorClient::onReadyRead() {
    if (!socket) return;

    receiveBuffer.append(QString::fromUtf8(socket->readAll()));

    int newline = receiveBuffer.indexOf('\n');
    while (newline >= 0) {
        const QString message = receiveBuffer.left(newline).trimmed();
        receiveBuffer.remove(0, newline + 1);

        if (!message.isEmpty()) {
            std::cout << "Received JSON: " << message.toStdString() << std::endl;
            emit dataReceived(message);
        }
        newline = receiveBuffer.indexOf('\n');
    }
}

void SimulatorClient::onError(QAbstractSocket::SocketError error) {
    Q_UNUSED(error);
    if (socket) {
        std::cout << "Socket error: " << socket->errorString().toStdString() << std::endl;
        emit errorOccurred(socket->errorString());
    }
}
