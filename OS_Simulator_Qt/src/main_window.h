#pragma once

#include <QMainWindow>
#include <QTcpSocket>
#include <QComboBox>
#include <QPushButton>
#include <QTableWidget>
#include <QChartView>
#include <QTabWidget>
#include <QLabel>
#include <QListWidget>
#include <QJsonObject>
#include <QVector>
#include <memory>

class GanttWidget;
class SimulatorClient;
class SyncTimelineWidget;
class PhilosophersWidget;
class QVariantAnimation;
class QResizeEvent;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onAlgorithmChanged(int index);
    void onStartSimulation();
    void onConnected();
    void onDisconnected();
    void onDataReceived(const QString &data);
    void onError(const QString &error);

private:
    void applyTheme();
    void setupEntranceAnimations();
    void updateResponsiveMode();
    void animateTabTransition(int index);
    void playButtonPulse(QPushButton *button);
    void setupUI();
    void createTabs();
    void createSchedulerTab();
    void createMetricsTab();
    void createSynchronizationTab();
    void createLogsTab();
    void createAboutTab();
    void updateGanttChart(const QJsonObject &jsonData);
    void updateMetricsTable(const QJsonObject &jsonData);
    void updateSynchronizationTab(const QJsonObject &jsonData);
    void animateMetricLabel(QLabel *label, double targetValue, int decimals, const QString &suffix = QString());
    void addLog(const QString &message);

    // UI Components
    QTabWidget *tabWidget;
    QComboBox *themeCombo;
    QVector<QWidget *> animatedCards;
    bool compactMode = false;
    QString activeTheme = "Cyber Blue";
    
    // Scheduler Tab
    QComboBox *algorithmCombo;
    QPushButton *startButton;
    GanttWidget *ganttChart;
    QTableWidget *metricsTable;
    QLabel *avgWaitValue;
    QLabel *avgTurnaroundValue;
    QLabel *throughputValue;
    QLabel *cpuUtilizationValue;

    // Synchronization Tab
    QComboBox *syncDemoCombo;
    QPushButton *startSyncButton;
    QListWidget *syncEventList;
    SyncTimelineWidget *syncTimeline;
    PhilosophersWidget *philosophersView;
    QLabel *raceResultLabel;
    
    // Logs Tab
    class QTextEdit *logsEdit;
    
    // Status
    QLabel *statusLabel;
    
    // Client
    std::unique_ptr<SimulatorClient> client;
};
