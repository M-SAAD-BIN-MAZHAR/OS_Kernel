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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), client(std::make_unique<SimulatorClient>())
{
    setWindowTitle("OS Process Scheduler Simulator");
    setGeometry(100, 100, 1200, 800);
    
    setupUI();
    
    // Connect client signals
    connect(client.get(), &SimulatorClient::connected, this, &MainWindow::onConnected);
    connect(client.get(), &SimulatorClient::disconnected, this, &MainWindow::onDisconnected);
    connect(client.get(), &SimulatorClient::dataReceived, this, &MainWindow::onDataReceived);
    connect(client.get(), &SimulatorClient::errorOccurred, this, &MainWindow::onError);
    
    // Attempt connection
    client->connectToServer();
    
    // Show status
    statusLabel = new QLabel("Disconnected from server", this);
    statusBar()->addWidget(statusLabel);
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
    algorithmCombo->addItems({"FCFS", "Round Robin", "Priority"});
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
    layout->addWidget(ganttChart);
    
    tabWidget->addTab(tab, "Scheduler");
}

void MainWindow::createMetricsTab() {
    QWidget *tab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(tab);
    
    metricsTable = new QTableWidget();
    metricsTable->setColumnCount(5);
    metricsTable->setHorizontalHeaderLabels({"Process", "Wait Time", "Turnaround Time", "CPU Burst", "Priority"});
    metricsTable->horizontalHeader()->setStretchLastSection(true);
    
    layout->addWidget(metricsTable);
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
    QTextEdit *logEdit = new QTextEdit();
    logEdit->setReadOnly(true);
    layout->addWidget(logEdit);
    tabWidget->addTab(tab, "Logs");
}

void MainWindow::createAboutTab() {
    QWidget *tab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(tab);
    layout->addWidget(new QLabel("OS Process Scheduler Simulator v1.0\n\nA simulator for CPU scheduling algorithms"));
    layout->addStretch();
    tabWidget->addTab(tab, "About");
}

void MainWindow::onAlgorithmChanged(int index) {
    QString algorithms[] = {"FCFS", "RoundRobin", "Priority"};
    if (client) {
        client->selectScheduler(algorithms[index]);
    }
}

void MainWindow::onStartSimulation() {
    if (client) {
        client->startSimulation();
    }
}

void MainWindow::onConnected() {
    statusLabel->setText("Connected to server");
}

void MainWindow::onDisconnected() {
    statusLabel->setText("Disconnected from server");
}

void MainWindow::onDataReceived(const QString &data) {
    // Handle JSON data from engine
    // Parse and update gantt chart and metrics
}

void MainWindow::onError(const QString &error) {
    statusLabel->setText("Error: " + error);
}
