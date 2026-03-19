#pragma once

#include <QChartView>
#include <QBarSeries>
#include <QBarCategoryAxis>
#include <QValueAxis>
#include <QBarSet>
#include <vector>
#include <string>

struct ProcessSchedule {
    std::string processName;
    int startTime;
    int endTime;
    std::string state;
};

class GanttWidget : public QChartView {
    Q_OBJECT

public:
    explicit GanttWidget(QWidget *parent = nullptr);
    
    // Update gantt chart with scheduling data
    void updateGanttChart(const std::vector<ProcessSchedule> &schedule);
    void clearChart();

private:
    void setupChart();
    void addScheduleBar(const std::string &processName, int startTime, int endTime, const std::string &state);
    
    QBarSeries *series;
    QBarCategoryAxis *axisX;
    QValueAxis *axisY;
};
