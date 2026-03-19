#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QString>
#include <memory>

class SimulatorClient : public QObject {
    Q_OBJECT

public:
    explicit SimulatorClient(const QString &host = "localhost", quint16 port = 9000, QObject *parent = nullptr);
    ~SimulatorClient();

    void connectToServer();
    void disconnectFromServer();
    bool isConnected() const;
    
    // Send commands to engine
    void selectScheduler(const QString &algorithm);
    void startSimulation();

signals:
    void connected();
    void disconnected();
    void dataReceived(const QString &jsonData);
    void errorOccurred(const QString &errorMessage);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError();

private:
    void sendCommand(const QString &command);

    std::unique_ptr<QTcpSocket> socket;
    QString host;
    quint16 port;
};
