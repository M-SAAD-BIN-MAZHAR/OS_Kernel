#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QListWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QJsonObject>

class DeadlockWidget : public QWidget {
    Q_OBJECT

public:
    explicit DeadlockWidget(QWidget *parent = nullptr);
    void updateDeadlockData(const QJsonObject &jsonData);

signals:
    void startDemoRequested(const QString &scenario);
    void stopDemoRequested();

private:
    QComboBox *scenarioCombo;
    QPushButton *startButton;
    QPushButton *stopButton;
    
    QLabel *statusLabel;
    QLabel *deadlockDetectedLabel;
    QLabel *safeSequenceLabel;
    
    QTableWidget *allocationTable;
    QTableWidget *requestTable;
    QTableWidget *availableTable;
    
    QListWidget *eventList;
    
    void setupUI();
};
