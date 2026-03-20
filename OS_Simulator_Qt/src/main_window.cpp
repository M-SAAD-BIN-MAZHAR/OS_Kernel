#include "main_window.h"
#include "simulator_client.h"
#include "gantt_widget.h"
#include "sync_timeline_widget.h"
#include "philosophers_widget.h"

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
#include <QListWidgetItem>
#include <QFrame>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QVariantAnimation>
#include <QResizeEvent>
#include <QTimer>
#include <iostream>
#include <cmath>

namespace {
#ifdef Q_OS_WIN
constexpr bool kUiSafeMode = true;
#else
constexpr bool kUiSafeMode = false;
#endif
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), client(std::make_unique<SimulatorClient>("localhost", 9000))
{
    setWindowTitle("OS Process Scheduler Simulator");
    setGeometry(100, 100, 1400, 900);
    setMinimumSize(900, 680);
    
    setupUI();
    applyTheme();
    updateResponsiveMode();
    setupEntranceAnimations();
    
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
    mainLayout->setContentsMargins(16, 16, 16, 12);
    mainLayout->setSpacing(12);

    auto *titleCard = new QFrame(this);
    titleCard->setObjectName("titleCard");
    auto *titleLayout = new QVBoxLayout(titleCard);
    titleLayout->setContentsMargins(20, 16, 20, 16);
    titleLayout->setSpacing(8);

    auto *topRow = new QHBoxLayout();

    auto *title = new QLabel("OS Process Scheduler Control Center", titleCard);
    title->setObjectName("dashboardTitle");
    topRow->addWidget(title);
    topRow->addStretch();

    auto *themeLabel = new QLabel("Theme:", titleCard);
    themeLabel->setObjectName("themeLabel");
    topRow->addWidget(themeLabel);

    themeCombo = new QComboBox(titleCard);
    themeCombo->addItems({"Cyber Blue", "Emerald Ops", "Light Pro"});
    themeCombo->setCurrentText(activeTheme);
    connect(themeCombo, &QComboBox::currentTextChanged, this, [this](const QString &themeName) {
        activeTheme = themeName;
        applyTheme();
    });
    topRow->addWidget(themeCombo);

    auto *subtitle = new QLabel("Live CPU scheduling, synchronization telemetry, and process analytics", titleCard);
    subtitle->setObjectName("dashboardSubtitle");

    titleLayout->addLayout(topRow);
    titleLayout->addWidget(subtitle);
    mainLayout->addWidget(titleCard);
    animatedCards.append(titleCard);
    
    tabWidget = new QTabWidget(this);
    tabWidget->setDocumentMode(true);
    connect(tabWidget, &QTabWidget::currentChanged, this, &MainWindow::animateTabTransition);
    
    createSchedulerTab();
    createMetricsTab();
    createSynchronizationTab();
    createLogsTab();
    createAboutTab();
    
    mainLayout->addWidget(tabWidget);
    setCentralWidget(centralWidget);
}

void MainWindow::createSchedulerTab() {
    QWidget *tab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(tab);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(10);
    
    // Algorithm selector
    auto *controlsCard = new QFrame(tab);
    controlsCard->setObjectName("panelCard");
    QHBoxLayout *selectLayout = new QHBoxLayout();
    selectLayout->setContentsMargins(14, 12, 14, 12);
    selectLayout->addWidget(new QLabel("Scheduling Algorithm:"));
    algorithmCombo = new QComboBox();
    algorithmCombo->addItems({"FCFS", "RoundRobin", "Priority"});
    connect(algorithmCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onAlgorithmChanged);
    selectLayout->addWidget(algorithmCombo);
    
    startButton = new QPushButton("Start Simulation");
    startButton->setObjectName("primaryButton");
    connect(startButton, &QPushButton::clicked, this, &MainWindow::onStartSimulation);
    connect(startButton, &QPushButton::clicked, this, [this]() { playButtonPulse(startButton); });
    selectLayout->addWidget(startButton);
    selectLayout->addStretch();

    controlsCard->setLayout(selectLayout);
    layout->addWidget(controlsCard);
    animatedCards.append(controlsCard);
    
    // Gantt chart
    auto *chartCard = new QFrame(tab);
    chartCard->setObjectName("panelCard");
    auto *chartLayout = new QVBoxLayout(chartCard);
    chartLayout->setContentsMargins(8, 8, 8, 8);
    ganttChart = new GanttWidget();
    ganttChart->setMinimumHeight(220);
    chartLayout->addWidget(ganttChart);
    layout->addWidget(chartCard);
    animatedCards.append(chartCard);
    
    tabWidget->addTab(tab, "Scheduler");
}

void MainWindow::createMetricsTab() {
    QWidget *tab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(tab);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(10);

    auto *summaryCard = new QFrame(tab);
    summaryCard->setObjectName("panelCard");
    QHBoxLayout *summaryLayout = new QHBoxLayout(summaryCard);
    summaryLayout->setContentsMargins(12, 10, 12, 10);
    summaryLayout->setSpacing(14);

    auto makeMetricTile = [&](const QString &titleText, QLabel **valueLabel) {
        auto *tile = new QFrame(summaryCard);
        tile->setObjectName("metricTile");
        auto *tileLayout = new QVBoxLayout(tile);
        tileLayout->setContentsMargins(10, 8, 10, 8);

        auto *title = new QLabel(titleText, tile);
        title->setObjectName("metricTitle");
        *valueLabel = new QLabel("--", tile);
        (*valueLabel)->setObjectName("metricValue");

        tileLayout->addWidget(title);
        tileLayout->addWidget(*valueLabel);
        summaryLayout->addWidget(tile);
    };

    makeMetricTile("Avg Wait Time", &avgWaitValue);
    makeMetricTile("Avg Turnaround", &avgTurnaroundValue);
    makeMetricTile("Throughput", &throughputValue);
    makeMetricTile("CPU Utilization", &cpuUtilizationValue);

    layout->addWidget(summaryCard);
    animatedCards.append(summaryCard);

    auto *tableCard = new QFrame(tab);
    tableCard->setObjectName("panelCard");
    auto *tableLayout = new QVBoxLayout(tableCard);
    tableLayout->setContentsMargins(10, 10, 10, 10);
    
    metricsTable = new QTableWidget();
    metricsTable->setColumnCount(6);
    metricsTable->setHorizontalHeaderLabels({
        "Process", "Burst Time", "Arrival Time", "Wait Time", "Turnaround Time", "Priority"
    });
    metricsTable->horizontalHeader()->setStretchLastSection(true);
    metricsTable->setAlternatingRowColors(true);
    metricsTable->verticalHeader()->setVisible(false);
    metricsTable->setShowGrid(false);
    
    auto *tableTitle = new QLabel("Per-Process Execution Metrics");
    tableTitle->setObjectName("sectionTitle");
    tableLayout->addWidget(tableTitle);
    tableLayout->addWidget(metricsTable);

    layout->addWidget(tableCard);
    animatedCards.append(tableCard);
    tabWidget->addTab(tab, "Metrics");
}

void MainWindow::createSynchronizationTab() {
    QWidget *tab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(tab);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(10);

    auto *controlsCard = new QFrame(tab);
    controlsCard->setObjectName("panelCard");
    QHBoxLayout *controls = new QHBoxLayout();
    controls->setContentsMargins(12, 10, 12, 10);
    controls->addWidget(new QLabel("Demo:"));

    syncDemoCombo = new QComboBox();
    syncDemoCombo->addItem("Producer-Consumer", "producer_consumer");
    syncDemoCombo->addItem("Dining Philosophers (Deadlock)", "philosophers_deadlock");
    syncDemoCombo->addItem("Dining Philosophers (Safe)", "philosophers_safe");
    syncDemoCombo->addItem("Race Condition", "race");
    controls->addWidget(syncDemoCombo);

    startSyncButton = new QPushButton("Start Sync Demo");
    startSyncButton->setObjectName("primaryButton");
    connect(startSyncButton, &QPushButton::clicked, this, [this]() {
        if (!client || !client->isConnected()) {
            addLog("Error: Not connected to server");
            return;
        }

        const QString demo = syncDemoCombo->currentData().toString();
        client->startSyncDemo(demo);
        addLog("Started sync demo: " + syncDemoCombo->currentText());
    });
    connect(startSyncButton, &QPushButton::clicked, this, [this]() { playButtonPulse(startSyncButton); });
    controls->addWidget(startSyncButton);
    controls->addStretch();
    controlsCard->setLayout(controls);
    layout->addWidget(controlsCard);
    animatedCards.append(controlsCard);

    raceResultLabel = new QLabel("Race Result: N/A");
    raceResultLabel->setObjectName("sectionTitle");
    layout->addWidget(raceResultLabel);

    auto *timelineCard = new QFrame(tab);
    timelineCard->setObjectName("panelCard");
    auto *timelineLayout = new QVBoxLayout(timelineCard);
    timelineLayout->setContentsMargins(10, 10, 10, 10);
    auto *timelineTitle = new QLabel("Thread Timeline");
    timelineTitle->setObjectName("sectionTitle");
    timelineLayout->addWidget(timelineTitle);
    syncTimeline = new SyncTimelineWidget();
    timelineLayout->addWidget(syncTimeline);
    layout->addWidget(timelineCard);
    animatedCards.append(timelineCard);

    auto *philosopherCard = new QFrame(tab);
    philosopherCard->setObjectName("panelCard");
    auto *philosopherLayout = new QVBoxLayout(philosopherCard);
    philosopherLayout->setContentsMargins(10, 10, 10, 10);
    auto *philosopherTitle = new QLabel("Dining Philosophers State");
    philosopherTitle->setObjectName("sectionTitle");
    philosopherLayout->addWidget(philosopherTitle);
    philosophersView = new PhilosophersWidget();
    philosopherLayout->addWidget(philosophersView);
    layout->addWidget(philosopherCard);
    animatedCards.append(philosopherCard);

    auto *logTitle = new QLabel("Synchronization Event Log");
    logTitle->setObjectName("sectionTitle");
    layout->addWidget(logTitle);
    syncEventList = new QListWidget();
    syncEventList->setMinimumHeight(120);
    layout->addWidget(syncEventList);
    animatedCards.append(syncEventList);

    tabWidget->addTab(tab, "Synchronization");
}

void MainWindow::createLogsTab() {
    QWidget *tab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(tab);
    layout->setContentsMargins(10, 10, 10, 10);
    logsEdit = new QTextEdit();
    logsEdit->setObjectName("logsPanel");
    logsEdit->setReadOnly(true);
    layout->addWidget(logsEdit);
    animatedCards.append(logsEdit);
    tabWidget->addTab(tab, "Logs");
}

void MainWindow::createAboutTab() {
    QWidget *tab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(tab);
    layout->setContentsMargins(14, 14, 14, 14);
    auto *aboutCard = new QFrame(tab);
    aboutCard->setObjectName("panelCard");
    auto *aboutLayout = new QVBoxLayout(aboutCard);
    aboutLayout->setContentsMargins(18, 14, 18, 14);

    auto *about = new QLabel("OS Process Scheduler Simulator v1.0\n\n"
                             "Advanced simulator for CPU scheduling, process synchronization, and runtime analytics.\n\n"
                             "Algorithms: FCFS, Round Robin, Priority\n"
                             "Synchronization Demos: Producer-Consumer, Dining Philosophers, Race Condition\n\n"
                             "© 2026");
    about->setWordWrap(true);
    aboutLayout->addWidget(about);

    layout->addWidget(aboutCard);
    animatedCards.append(aboutCard);
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
        addLog("JSON parse error: " + parseError.errorString());
        return;
    }

    const QJsonObject jsonData = doc.object();
    const QString messageType = jsonData.value("type").toString("scheduler");

    if (messageType == "sync") {
        updateSynchronizationTab(jsonData);
    } else {
        updateGanttChart(jsonData);
        updateMetricsTable(jsonData);
    }
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

    if (jsonData.contains("metrics") && jsonData.value("metrics").isObject()) {
        const QJsonObject metrics = jsonData.value("metrics").toObject();
        animateMetricLabel(avgWaitValue, metrics.value("avgWaitTime").toDouble(), 2);
        animateMetricLabel(avgTurnaroundValue, metrics.value("avgTurnaroundTime").toDouble(), 2);
        animateMetricLabel(throughputValue, metrics.value("throughput").toDouble(), 2);
        animateMetricLabel(cpuUtilizationValue, metrics.value("cpuUtilization").toDouble(), 2, "%");
    }
}

void MainWindow::addLog(const QString &message) {
    if (logsEdit) {
        logsEdit->append("[" + QTime::currentTime().toString() + "] " + message);
    }
}

void MainWindow::updateSynchronizationTab(const QJsonObject &jsonData) {
    if (jsonData.contains("events") && jsonData.value("events").isArray()) {
        const QJsonArray events = jsonData.value("events").toArray();

        syncEventList->clear();
        for (const QJsonValue &value : events) {
            if (!value.isObject()) {
                continue;
            }

            const QJsonObject event = value.toObject();
            const QString actor = event.value("actor").toString();
            const QString action = event.value("action").toString();
            const QString resource = event.value("resource").toString();
            const QString status = event.value("status").toString();
            const int tick = event.value("tick").toInt();

            const QString text = QString("t=%1 | %2 %3 %4 (%5)")
                                     .arg(tick)
                                     .arg(actor)
                                     .arg(action)
                                     .arg(resource)
                                     .arg(status);

            auto *item = new QListWidgetItem(text);
            if (status.contains("blocked", Qt::CaseInsensitive) || status.contains("stalled", Qt::CaseInsensitive)) {
                item->setForeground(QColor("#dc2626"));
            } else if (action.contains("release", Qt::CaseInsensitive) || action.contains("signal", Qt::CaseInsensitive)) {
                item->setForeground(QColor("#2563eb"));
            } else if (action.contains("acquire", Qt::CaseInsensitive) || action.contains("wait", Qt::CaseInsensitive)) {
                item->setForeground(QColor("#16a34a"));
            }
            syncEventList->addItem(item);
        }
    }

    if (jsonData.contains("timeline") && jsonData.value("timeline").isArray()) {
        std::vector<SyncTimelineSpan> spans;
        const QJsonArray timeline = jsonData.value("timeline").toArray();
        for (const QJsonValue &value : timeline) {
            if (!value.isObject()) {
                continue;
            }
            const QJsonObject span = value.toObject();
            SyncTimelineSpan timelineSpan;
            timelineSpan.actor = span.value("actor").toString();
            timelineSpan.startTick = span.value("startTick").toInt();
            timelineSpan.endTick = span.value("endTick").toInt();
            timelineSpan.state = span.value("state").toString();
            spans.push_back(timelineSpan);
        }
        syncTimeline->setTimeline(spans);
    }

    if (jsonData.contains("philosophers") && jsonData.value("philosophers").isArray()) {
        std::vector<QString> states(5, "THINKING");
        const QJsonArray philosophers = jsonData.value("philosophers").toArray();
        for (const QJsonValue &value : philosophers) {
            if (!value.isObject()) {
                continue;
            }
            const QJsonObject philosopher = value.toObject();
            const int id = philosopher.value("id").toInt();
            if (id >= 0 && id < 5) {
                states[id] = philosopher.value("state").toString("THINKING");
            }
        }
        philosophersView->setStates(states);
    }

    if (jsonData.contains("race") && jsonData.value("race").isObject()) {
        const QJsonObject race = jsonData.value("race").toObject();
        const int noMutex = race.value("withoutMutex").toInt();
        const int withMutex = race.value("withMutex").toInt();
        const int target = race.value("target").toInt();

        raceResultLabel->setText(QString("Race Result: without mutex=%1, with mutex=%2, target=%3")
                                     .arg(noMutex)
                                     .arg(withMutex)
                                     .arg(target));
    }
}

void MainWindow::animateMetricLabel(QLabel *label, double targetValue, int decimals, const QString &suffix) {
    if (!label) {
        return;
    }

    if (kUiSafeMode) {
        label->setText(QString::number(targetValue, 'f', decimals) + suffix);
        return;
    }

    bool ok = false;
    QString currentText = label->text();
    if (!suffix.isEmpty() && currentText.endsWith(suffix)) {
        currentText.chop(suffix.size());
    }
    const double startValue = currentText.toDouble(&ok);
    const double safeStartValue = ok ? startValue : 0.0;

    if (std::abs(safeStartValue - targetValue) < 0.0001) {
        label->setText(QString::number(targetValue, 'f', decimals) + suffix);
        return;
    }

    auto *animation = new QVariantAnimation(label);
    animation->setDuration(420);
    animation->setStartValue(safeStartValue);
    animation->setEndValue(targetValue);
    animation->setEasingCurve(QEasingCurve::OutCubic);

    connect(animation, &QVariantAnimation::valueChanged, label, [label, decimals, suffix](const QVariant &value) {
        label->setText(QString::number(value.toDouble(), 'f', decimals) + suffix);
    });

    connect(animation, &QVariantAnimation::finished, animation, &QObject::deleteLater);
    animation->start();
}

void MainWindow::applyTheme() {
    const bool isLight = (activeTheme == "Light Pro");

    QString bgStart = "#0b1320";
    QString bgMid = "#111827";
    QString bgEnd = "#1f2937";
    QString accent = "#0ea5e9";
    QString accentSoft = "#22d3ee";
    QString accentText = "#082f49";
    QString panelBg = "#111827";
    QString textMain = "#e2e8f0";
    QString border = "#334155";
    QString tabBg = "#1f2937";
    QString tableBg = "#0b1220";
    QString titleText = "#f8fafc";

    if (activeTheme == "Emerald Ops") {
        bgStart = "#052e2b";
        bgMid = "#064e3b";
        bgEnd = "#022c22";
        accent = "#34d399";
        accentSoft = "#6ee7b7";
        accentText = "#03251b";
        panelBg = "#05372c";
        textMain = "#d1fae5";
        border = "#1f6c58";
        tabBg = "#0b3b30";
        tableBg = "#072e26";
        titleText = "#ecfdf5";
    } else if (isLight) {
        bgStart = "#ecfeff";
        bgMid = "#f8fafc";
        bgEnd = "#e2e8f0";
        accent = "#0284c7";
        accentSoft = "#38bdf8";
        accentText = "#f0f9ff";
        panelBg = "#ffffff";
        textMain = "#0f172a";
        border = "#cbd5e1";
        tabBg = "#e2e8f0";
        tableBg = "#f8fafc";
        titleText = "#0f172a";
    }

    const QString compactPadding = compactMode ? "6px 10px" : "8px 16px";
    const QString compactFont = compactMode ? "12px" : "13px";
    const QString titleFont = compactMode ? "20px" : "24px";

    setStyleSheet(QString(R"(
        QMainWindow {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 %1, stop:0.6 %2, stop:1 %3);
        }
        #titleCard {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 %2, stop:1 %3);
            border: 1px solid %9;
            border-radius: 14px;
        }
        #dashboardTitle {
            color: %13;
            font-size: %14;
            font-weight: 700;
            letter-spacing: 0.4px;
        }
        #dashboardSubtitle {
            color: %4;
            font-size: %12;
        }
        #themeLabel { color: %10; }
        QTabWidget::pane {
            border: 1px solid %9;
            background: %7;
            border-radius: 10px;
            top: -1px;
        }
        QTabBar::tab {
            background: %10;
            color: %8;
            padding: %11;
            border: 1px solid %9;
            border-top-left-radius: 8px;
            border-top-right-radius: 8px;
            margin-right: 4px;
        }
        QTabBar::tab:selected {
            background: %4;
            color: %6;
            border-color: %5;
            font-weight: 700;
        }
        QTabBar::tab:hover {
            background: %9;
        }
        #panelCard {
            background: %7;
            border: 1px solid %9;
            border-radius: 12px;
        }
        QLabel {
            color: %8;
            font-size: %12;
        }
        #sectionTitle {
            color: %13;
            font-size: 15px;
            font-weight: 700;
        }
        QComboBox {
            background: %10;
            color: %8;
            border: 1px solid %9;
            border-radius: 8px;
            padding: 6px 10px;
            min-width: 150px;
        }
        QComboBox::drop-down {
            border: 0px;
            width: 24px;
        }
        QPushButton {
            background: %10;
            color: %8;
            border: 1px solid %9;
            border-radius: 8px;
            padding: 7px 14px;
            font-weight: 600;
        }
        QPushButton:hover {
            background: %9;
        }
        QPushButton:pressed {
            background: %3;
        }
        #primaryButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 %5, stop:1 %4);
            color: %6;
            border: 1px solid %5;
        }
        #primaryButton:hover {
            background: %5;
        }
        QTableWidget {
            background: %15;
            color: %8;
            border: 1px solid %9;
            border-radius: 10px;
            alternate-background-color: %7;
            gridline-color: %10;
            selection-background-color: %4;
            selection-color: %6;
        }
        QHeaderView::section {
            background: %10;
            color: %5;
            padding: 7px;
            border: 0px;
            font-weight: 700;
        }
        QListWidget {
            background: %15;
            color: %8;
            border: 1px solid %9;
            border-radius: 10px;
            padding: 6px;
        }
        QListWidget::item {
            padding: 6px;
            border-radius: 6px;
        }
        QListWidget::item:selected {
            background: %10;
        }
        #metricTile {
            background: %15;
            border: 1px solid %9;
            border-radius: 10px;
        }
        #metricTitle {
            color: %5;
            font-size: 12px;
            font-weight: 600;
        }
        #metricValue {
            color: %13;
            font-size: 20px;
            font-weight: 700;
        }
        #logsPanel {
            background: %15;
            color: %8;
            border: 1px solid %9;
            border-radius: 10px;
            padding: 6px;
            font-family: Consolas, 'Courier New', monospace;
        }
        QStatusBar {
            background: %15;
            color: %5;
            border-top: 1px solid %9;
        }
    )")
    .arg(bgStart)
    .arg(bgMid)
    .arg(bgEnd)
    .arg(accent)
    .arg(accentSoft)
    .arg(accentText)
    .arg(panelBg)
    .arg(textMain)
    .arg(border)
    .arg(tabBg)
    .arg(compactPadding)
    .arg(compactFont)
    .arg(titleText)
    .arg(titleFont)
    .arg(tableBg));
}

void MainWindow::setupEntranceAnimations() {
    if (kUiSafeMode) {
        return;
    }

    int delay = 0;
    for (QWidget *widget : animatedCards) {
        if (!widget) {
            continue;
        }

        auto *effect = new QGraphicsOpacityEffect(widget);
        effect->setOpacity(0.0);
        widget->setGraphicsEffect(effect);

        auto *animation = new QPropertyAnimation(effect, "opacity", widget);
        animation->setDuration(380);
        animation->setStartValue(0.0);
        animation->setEndValue(1.0);

        QTimer::singleShot(delay, this, [animation]() {
            animation->start(QAbstractAnimation::DeleteWhenStopped);
        });
        delay += 70;
    }
}

void MainWindow::updateResponsiveMode() {
    const bool newCompact = width() < 1180;
    if (newCompact == compactMode) {
        return;
    }

    compactMode = newCompact;
    applyTheme();
}

void MainWindow::animateTabTransition(int index) {
    if (kUiSafeMode) {
        return;
    }

    if (!tabWidget) {
        return;
    }

    QWidget *page = tabWidget->widget(index);
    if (!page) {
        return;
    }

    auto *effect = new QGraphicsOpacityEffect(page);
    effect->setOpacity(0.0);
    page->setGraphicsEffect(effect);

    auto *fade = new QPropertyAnimation(effect, "opacity", page);
    fade->setDuration(210);
    fade->setStartValue(0.0);
    fade->setEndValue(1.0);
    fade->setEasingCurve(QEasingCurve::OutCubic);

    // QWidget owns and deletes the active graphics effect; avoid manual delete.
    connect(fade, &QPropertyAnimation::finished, page, [page]() {
        page->setGraphicsEffect(nullptr);
    });
    fade->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::playButtonPulse(QPushButton *button) {
    if (kUiSafeMode) {
        return;
    }

    if (!button) {
        return;
    }

    auto *effect = qobject_cast<QGraphicsOpacityEffect *>(button->graphicsEffect());
    if (!effect) {
        effect = new QGraphicsOpacityEffect(button);
        effect->setOpacity(1.0);
        button->setGraphicsEffect(effect);
    }

    auto *fadeDown = new QPropertyAnimation(effect, "opacity", button);
    fadeDown->setDuration(70);
    fadeDown->setStartValue(1.0);
    fadeDown->setEndValue(0.75);

    auto *fadeUp = new QPropertyAnimation(effect, "opacity", button);
    fadeUp->setDuration(130);
    fadeUp->setStartValue(0.75);
    fadeUp->setEndValue(1.0);

    auto *sequence = new QSequentialAnimationGroup(button);
    sequence->addAnimation(fadeDown);
    sequence->addAnimation(fadeUp);
    connect(sequence, &QSequentialAnimationGroup::finished, sequence, &QObject::deleteLater);
    sequence->start();
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    updateResponsiveMode();
}
