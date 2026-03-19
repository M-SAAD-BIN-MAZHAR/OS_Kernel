#include "main_window.h"
#include "simulator_client.h"
#include "gantt_widget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>
#include <QStatusBar>
#include <QTextEdit>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTime>
#include <QTimer>
#include <iostream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), client(std::make_unique<SimulatorClient>("172.21.89.117", 9000))
{
    setWindowTitle("OS Process Scheduler Simulator");
    setGeometry(100, 100, 1400, 900);
    
    setupUI();
    
    // Connect client signals
    connect(client.get(), &SimulatorClient::connected, this, &MainWindow::onConnected);
    connect(client.get(), &SimulatorClient::disconnected, this, &MainWindow::onDisconnected);
    connect(client.get(), &SimulatorClient::dataReceived, this, &MainWindow::onDataReceived);
    connect(client.get(), &SimulatorClient::errorOccurred, this, &MainWindow::onError);
    
    // Show status
    statusLabel = new QLabel("Connecting to server...", this);
    statusBar()->addWidget(statusLabel);
    
    // Attempt connection
    client->connectToServer();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI() {
    // Create central widget with tab layout
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    
    tabWidget = new QTabWidget(this);
    
    createSchedulerTab();
    createMetricsTab();
    createSettingsTab();
    createLogsTab();
    createAboutTab();
    
    mainLayout->addWidget(tabWidget);
    setCentralWidget(centralWidget);
}

void MainWindow::createSchedulerTab() {
    QWidget *tab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(tab);
    
    // Algorithm selector
    QHBoxLayout *selectLayout = new QHBoxLayout();
    selectLayout->addWidget(new QLabel("Scheduling Algorithm:"));
    algorithmCombo = new QComboBox();
    algorithmCombo->addItems({"FCFS", "RoundRobin", "Priority"});
    connect(algorithmCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onAlgorithmChanged);
    selectLayout->addWidget(algorithmCombo);
    
    startButton = new QPushButton("Start Simulation");
    connect(startButton, &QPushButton::clicked, this, &MainWindow::onStartSimulation);
    selectLayout->addWidget(startButton);
    selectLayout->addStretch();
    
    layout->addLayout(selectLayout);
    
    // Gantt chart
    ganttChart = new GanttWidget();
    ganttChart->setMinimumHeight(300);
    layout->addWidget(ganttChart);
    
    tabWidget->addTab(tab, "Scheduler");
}

void MainWindow::createMetricsTab() {
    QWidget *tab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(tab);
    
    metricsTable = new QTableWidget();
    metricsTable->setColumnCount(6);
    metricsTable->setHorizontalHeaderLabels({
        "Process", "Burst Time", "Arrival Time", "Wait Time", "Turnaround Time", "Priority"
    });
    metricsTable->horizontalHeader()->setStretchLastSection(true);
    metricsTable->setAlternatingRowColors(true);
    
    layout->addWidget(new QLabel("Process Metrics:"));
    layout->addWidget(metricsTable);
    
    // Summary metrics
    QHBoxLayout *summaryLayout = new QHBoxLayout();
    summaryLayout->addWidget(new QLabel("Avg Wait Time: "));
    summaryLayout->addWidget(new QLabel("N/A"));
    summaryLayout->addWidget(new QLabel("Avg Turnaround Time: "));
    summaryLayout->addWidget(new QLabel("N/A"));
    summaryLayout->addWidget(new QLabel("Throughput: "));
    summaryLayout->addWidget(new QLabel("N/A"));
    summaryLayout->addStretch();
    
    layout->addLayout(summaryLayout);
    tabWidget->addTab(tab, "Metrics");
}

void MainWindow::createSettingsTab() {
    QWidget *tab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(tab);
    layout->addWidget(new QLabel("Settings will be configured here"));
    layout->addStretch();
    tabWidget->addTab(tab, "Settings");
}

void MainWindow::createLogsTab() {
    QWidget *tab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(tab);
    logsEdit = new QTextEdit();
    logsEdit->setReadOnly(true);
    layout->addWidget(logsEdit);
    tabWidget->addTab(tab, "Logs");
}

void MainWindow::createAboutTab() {
    QWidget *tab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(tab);
    layout->addWidget(new QLabel("OS Process Scheduler Simulator v1.0\n\n"
                                 "A simulator for CPU scheduling algorithms\n\n"
                                 "Supports: FCFS, Round Robin, Priority\n\n"
                                 "© 2026"));
    layout->addStretch();
    tabWidget->addTab(tab, "About");
}

void MainWindow::onAlgorithmChanged(int index) {
    QString algorithms[] = {"FCFS", "RoundRobin", "Priority"};
    if (client && client->isConnected()) {
        addLog("Selected algorithm: " + algorithms[index]);
        client->selectScheduler(algorithms[index]);
    } else {
        addLog("Warning: Not connected to server");
    }
}

void MainWindow::onStartSimulation() {
    if (client && client->isConnected()) {
        addLog("Started simulation");
        client->startSimulation();
    } else {
        addLog("Error: Not connected to server");
    }
}

void MainWindow::onConnected() {
    statusLabel->setText("Connected to server");
    addLog("Connected to scheduling engine");
}

void MainWindow::onDisconnected() {
    statusLabel->setText("Disconnected from server (Reconnecting...)");
    addLog("Disconnected from server - attempting reconnection");
    // Try to reconnect after 2 seconds
    QTimer::singleShot(2000, client.get(), &SimulatorClient::connectToServer);
}

void MainWindow::onDataReceived(const QString &data) {
    const QByteArray payload = data.toUtf8();
    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(payload, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        std::cerr << "JSON parse error: " << parseError.errorString().toStdString() << std::endl;
        return;
    }

    const QJsonObject jsonData = doc.object();
    updateGanttChart(jsonData);
    updateMetricsTable(jsonData);
}

void MainWindow::onError(const QString &error) {
    statusLabel->setText("Error: " + error);
    addLog("ERROR: " + error);
}

void MainWindow::updateGanttChart(const QJsonObject &jsonData) {
    if (!jsonData.contains("ganttChart") || !jsonData.value("ganttChart").isArray()) {
        return;
    }

    std::vector<ProcessSchedule> schedule;
    const QJsonArray ganttArray = jsonData.value("ganttChart").toArray();
    for (const QJsonValue &value : ganttArray) {
        if (!value.isObject()) {
            continue;
        }

        const QJsonObject item = value.toObject();
        ProcessSchedule ps;
        ps.processName = item.value("processName").toString().toStdString();
        ps.startTime = item.value("startTime").toInt();
        ps.endTime = item.value("endTime").toInt();
        ps.state = "RUNNING";
        schedule.push_back(ps);
    }

    ganttChart->updateGanttChart(schedule);
}

void MainWindow::updateMetricsTable(const QJsonObject &jsonData) {
    if (!jsonData.contains("processes") || !jsonData.value("processes").isArray()) {
        return;
    }

    metricsTable->setRowCount(0);

    int row = 0;
    const QJsonArray processes = jsonData.value("processes").toArray();
    for (const QJsonValue &value : processes) {
        if (!value.isObject()) {
            continue;
        }

        const QJsonObject proc = value.toObject();
        metricsTable->insertRow(row);

        metricsTable->setItem(row, 0, new QTableWidgetItem(proc.value("name").toString()));
        metricsTable->setItem(row, 1, new QTableWidgetItem(QString::number(proc.value("burstTime").toInt())));
        metricsTable->setItem(row, 2, new QTableWidgetItem(QString::number(proc.value("arrivalTime").toInt())));
        metricsTable->setItem(row, 3, new QTableWidgetItem(QString::number(proc.value("waitTime").toInt())));
        metricsTable->setItem(row, 4, new QTableWidgetItem(QString::number(proc.value("turnaroundTime").toInt())));
        metricsTable->setItem(row, 5, new QTableWidgetItem(QString::number(proc.value("priority").toInt())));

        row++;
    }
}

void MainWindow::addLog(const QString &message) {
    if (logsEdit) {
        logsEdit->append("[" + QTime::currentTime().toString() + "] " + message);
    }
}
