#include "deadlock_widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QJsonArray>
#include <QTableWidgetItem>
#include <QListWidgetItem>

DeadlockWidget::DeadlockWidget(QWidget *parent)
    : QWidget(parent) {
    setupUI();
}

void DeadlockWidget::setupUI() {
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);
    
    // Control panel
    auto *controlLayout = new QHBoxLayout();
    controlLayout->addWidget(new QLabel("Scenario:"));
    
    scenarioCombo = new QComboBox();
    scenarioCombo->addItems({"Safe State", "Deadlock", "Banker's Algorithm", "Prevention"});
    controlLayout->addWidget(scenarioCombo);
    
    startButton = new QPushButton("Start Demo");
    stopButton = new QPushButton("Stop Demo");
    controlLayout->addWidget(startButton);
    controlLayout->addWidget(stopButton);
    controlLayout->addStretch();

    connect(startButton, &QPushButton::clicked, this, [this]() {
        static const QStringList scenarioKeys = {"safe", "deadlock", "bankers", "prevention"};
        const int index = scenarioCombo->currentIndex();
        const QString scenario = (index >= 0 && index < scenarioKeys.size())
                                     ? scenarioKeys[index]
                                     : QString("safe");
        emit startDemoRequested(scenario);
    });

    connect(stopButton, &QPushButton::clicked, this, [this]() {
        emit stopDemoRequested();
    });
    
    mainLayout->addLayout(controlLayout);
    
    // Status indicators
    auto *statusLayout = new QHBoxLayout();
    statusLayout->addWidget(new QLabel("Status:"));
    statusLabel = new QLabel("Idle");
    statusLabel->setStyleSheet("color: #16a34a;");
    statusLayout->addWidget(statusLabel);
    statusLayout->addSpacing(20);
    
    statusLayout->addWidget(new QLabel("Deadlock:"));
    deadlockDetectedLabel = new QLabel("No");
    deadlockDetectedLabel->setStyleSheet("color: #16a34a;");
    statusLayout->addWidget(deadlockDetectedLabel);
    statusLayout->addSpacing(20);
    
    statusLayout->addWidget(new QLabel("Safe Sequence:"));
    safeSequenceLabel = new QLabel("Available");
    safeSequenceLabel->setStyleSheet("color: #16a34a;");
    statusLayout->addWidget(safeSequenceLabel);
    statusLayout->addStretch();
    
    mainLayout->addLayout(statusLayout);
    
    // Tables for allocation, request, available
    auto *tablesLayout = new QHBoxLayout();
    
    // Allocation table
    auto *allocLayout = new QVBoxLayout();
    allocLayout->addWidget(new QLabel("Allocation Matrix"));
    allocationTable = new QTableWidget();
    allocationTable->setColumnCount(6);
    allocationTable->setRowCount(5);
    allocationTable->setHorizontalHeaderLabels({"R0", "R1", "R2", "R3", "R4", "R5"});
    allocationTable->setVerticalHeaderLabels({"P0", "P1", "P2", "P3", "P4"});
    allocationTable->horizontalHeader()->setStretchLastSection(true);
    allocationTable->setMaximumHeight(150);
    allocLayout->addWidget(allocationTable);
    tablesLayout->addLayout(allocLayout);
    
    // Request table
    auto *reqLayout = new QVBoxLayout();
    reqLayout->addWidget(new QLabel("Request Matrix"));
    requestTable = new QTableWidget();
    requestTable->setColumnCount(6);
    requestTable->setRowCount(5);
    requestTable->setHorizontalHeaderLabels({"R0", "R1", "R2", "R3", "R4", "R5"});
    requestTable->setVerticalHeaderLabels({"P0", "P1", "P2", "P3", "P4"});
    requestTable->horizontalHeader()->setStretchLastSection(true);
    requestTable->setMaximumHeight(150);
    reqLayout->addWidget(requestTable);
    tablesLayout->addLayout(reqLayout);
    
    // Available resources
    auto *availLayout = new QVBoxLayout();
    availLayout->addWidget(new QLabel("Available Resources"));
    availableTable = new QTableWidget();
    availableTable->setColumnCount(6);
    availableTable->setRowCount(1);
    availableTable->setHorizontalHeaderLabels({"R0", "R1", "R2", "R3", "R4", "R5"});
    availableTable->setVerticalHeaderLabels({"Available"});
    availableTable->horizontalHeader()->setStretchLastSection(true);
    availableTable->setMaximumHeight(80);
    availLayout->addWidget(availableTable);
    tablesLayout->addLayout(availLayout);
    
    mainLayout->addLayout(tablesLayout);
    
    // Event log
    mainLayout->addWidget(new QLabel("Event Log"));
    eventList = new QListWidget();
    eventList->setMinimumHeight(150);
    mainLayout->addWidget(eventList);
    
    setLayout(mainLayout);
}

void DeadlockWidget::updateDeadlockData(const QJsonObject &jsonData) {
    // Update deadlock status
    bool detected = jsonData.value("deadlock").toObject().value("detected").toBool();
    deadlockDetectedLabel->setText(detected ? "Yes" : "No");
    deadlockDetectedLabel->setStyleSheet(detected ? "color: #dc2626;" : "color: #16a34a;");
    
    // Update safe sequence status
    bool safeExists = jsonData.value("safeSequence").toObject().value("exists").toBool();
    safeSequenceLabel->setText(safeExists ? "Available" : "None");
    safeSequenceLabel->setStyleSheet(safeExists ? "color: #16a34a;" : "color: #dc2626;");
    
    // Update allocation matrix
    const QJsonArray allocArray = jsonData.value("allocation").toArray();
    for (int i = 0; i < allocArray.size() && i < 5; ++i) {
        const QJsonArray row = allocArray[i].toArray();
        for (int j = 0; j < row.size() && j < 6; ++j) {
            auto *item = new QTableWidgetItem(QString::number(row[j].toInt()));
            item->setTextAlignment(Qt::AlignCenter);
            allocationTable->setItem(i, j, item);
        }
    }
    
    // Update request matrix
    const QJsonArray requestArray = jsonData.value("request").toArray();
    for (int i = 0; i < requestArray.size() && i < 5; ++i) {
        const QJsonArray row = requestArray[i].toArray();
        for (int j = 0; j < row.size() && j < 6; ++j) {
            auto *item = new QTableWidgetItem(QString::number(row[j].toInt()));
            item->setTextAlignment(Qt::AlignCenter);
            requestTable->setItem(i, j, item);
        }
    }
    
    // Update available resources
    const QJsonArray availArray = jsonData.value("available").toArray();
    for (int i = 0; i < availArray.size() && i < 6; ++i) {
        auto *item = new QTableWidgetItem(QString::number(availArray[i].toInt()));
        item->setTextAlignment(Qt::AlignCenter);
        availableTable->setItem(0, i, item);
    }
    
    // Update event log
    eventList->clear();
    const QJsonArray events = jsonData.value("events").toArray();
    for (const auto &event : events) {
        if (!event.isObject()) continue;
        
        const QJsonObject obj = event.toObject();
        const QString text = QString("t=%1 | P%2 | %3 | %4")
            .arg(obj.value("tick").toInt())
            .arg(obj.value("processId").toInt())
            .arg(obj.value("action").toString())
            .arg(obj.value("detail").toString());
        
        auto *item = new QListWidgetItem(text);
        if (!obj.value("success").toBool()) {
            item->setForeground(Qt::red);
        }
        eventList->addItem(item);
    }
}
