#include "gantt_widget.h"
#include <QChart>
#include <QBarCategoryAxis>
#include <QValueAxis>
#include <QBarSet>
#include <QPainter>
#include <QColor>
#include <algorithm>

GanttWidget::GanttWidget(QWidget *parent)
    : QChartView(parent), series(nullptr), axisX(nullptr), axisY(nullptr)
{
    setupChart();
}

void GanttWidget::setupChart() {
    QChart *chart = new QChart();
    chart->setTitle("CPU Scheduling Gantt Chart");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->setMargins(QMargins(10, 10, 10, 10));
    
    series = new QBarSeries();
    chart->addSeries(series);
    
    // Create axes
    axisX = new QBarCategoryAxis();
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
    
    axisY = new QValueAxis();
    axisY->setLabelFormat("%i ms");
    axisY->setRange(0, 20);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
    
    setChart(chart);
    setRenderHint(QPainter::Antialiasing);
}

void GanttWidget::updateGanttChart(const std::vector<ProcessSchedule> &schedule) {
    if (schedule.empty()) return;
    
    clearChart();
    
    QStringList categories;
    int maxTime = 0;
    
    // Collect all process names and find max time
    for (const auto &process : schedule) {
        if (std::find(categories.begin(), categories.end(), 
                     QString::fromStdString(process.processName)) == categories.end()) {
            categories.append(QString::fromStdString(process.processName));
        }
        maxTime = std::max(maxTime, process.endTime);
    }
    
    axisX->setCategories(categories);
    if (axisY) {
        axisY->setRange(0, maxTime + 5);
    }
    
    // Create bars for each process based on their duration
    for (const auto &process : schedule) {
        addScheduleBar(process.processName, process.startTime, process.endTime, process.state);
    }
}

void GanttWidget::clearChart() {
    if (series) {
        series->clear();
    }
    if (axisX) {
        axisX->clear();
    }
}

void GanttWidget::addScheduleBar(const std::string &processName, int startTime, int endTime, const std::string &state) {
    if (!series) return;
    
    int duration = endTime - startTime;
    
    QBarSet *set = new QBarSet(QString::fromStdString(processName) +
                              QString(" [%1-%2ms]").arg(startTime).arg(endTime));
    
    // Color by state
    if (state == "RUNNING") {
        set->setColor(QColor(76, 175, 80));  // Green
    } else if (state == "READY") {
        set->setColor(QColor(33, 150, 243)); // Blue
    } else if (state == "BLOCKED") {
        set->setColor(QColor(255, 193, 7));  // Yellow
    } else {
        set->setColor(QColor(200, 200, 200)); // Gray
    }
    
    *set << duration;
    series->append(set);
}
