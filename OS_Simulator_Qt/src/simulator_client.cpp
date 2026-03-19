#include "simulator_client.h"
#include <QJsonDocument>
#include <QJsonObject>

SimulatorClient::SimulatorClient(const QString &host, quint16 port, QObject *parent)
    : QObject(parent), socket(std::make_unique<QTcpSocket>()), host(host), port(port)
{
    connect(socket.get(), &QTcpSocket::connected, this, &SimulatorClient::onConnected);
    connect(socket.get(), &QTcpSocket::disconnected, this, &SimulatorClient::onDisconnected);
    connect(socket.get(), &QTcpSocket::readyRead, this, &SimulatorClient::onReadyRead);
    connect(socket.get(), QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error),
            this, &SimulatorClient::onError);
}

SimulatorClient::~SimulatorClient() {
    disconnectFromServer();
}

void SimulatorClient::connectToServer() {
    if (!socket->isOpen()) {
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

void SimulatorClient::sendCommand(const QString &command) {
    if (socket && socket->isOpen()) {
        socket->write(command.toUtf8());
        socket->flush();
    }
}

void SimulatorClient::onConnected() {
    emit connected();
}

void SimulatorClient::onDisconnected() {
    emit disconnected();
}

void SimulatorClient::onReadyRead() {
    if (!socket) return;
    
    QByteArray data = socket->readAll();
    QString jsonData = QString::fromUtf8(data);
    emit dataReceived(jsonData);
}

void SimulatorClient::onError() {
    if (socket) {
        emit errorOccurred(socket->errorString());
    }
}
