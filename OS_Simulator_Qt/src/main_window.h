#pragma once

#include <QMainWindow>
#include <QTcpSocket>
#include <QComboBox>
#include <QPushButton>
#include <QTableWidget>
#include <QChartView>
#include <QTabWidget>
#include <memory>

class GanttWidget;
class SimulatorClient;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onAlgorithmChanged(int index);
    void onStartSimulation();
    void onConnected();
    void onDisconnected();
    void onDataReceived(const QString &data);
    void onError(const QString &error);

private:
    void setupUI();
    void createTabs();
    void createSchedulerTab();
    void createMetricsTab();
    void createSettingsTab();
    void createLogsTab();
    void createAboutTab();

    // UI Components
    QTabWidget *tabWidget;
    
    // Scheduler Tab
    QComboBox *algorithmCombo;
    QPushButton *startButton;
    GanttWidget *ganttChart;
    QTableWidget *metricsTable;
    
    // Status
    QLabel *statusLabel;
    
    // Client
    std::unique_ptr<SimulatorClient> client;
};
