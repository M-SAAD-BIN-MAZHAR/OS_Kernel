#include "gantt_widget.h"
#include <QChart>
#include <QColor>

GanttWidget::GanttWidget(QWidget *parent)
    : QChartView(parent), series(nullptr), axisX(nullptr), axisY(nullptr)
{
    setupChart();
}

void GanttWidget::setupChart() {
    QChart *chart = new QChart();
    chart->setTitle("CPU Scheduling Gantt Chart");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    
    series = new QBarSeries();
    chart->addSeries(series);
    
    // Create axes
    axisX = new QBarCategoryAxis();
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
    
    axisY = new QValueAxis();
    axisY->setLabelFormat("%i ms");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
    
    setChart(chart);
    setRenderHint(QPainter::Antialiasing);
}

void GanttWidget::updateGanttChart(const std::vector<ProcessSchedule> &schedule) {
    clearChart();
    
    for (const auto &process : schedule) {
        addScheduleBar(process.processName, process.startTime, process.endTime, process.state);
    }
}

void GanttWidget::clearChart() {
    if (series) {
        series->clear();
    }
}

void GanttWidget::addScheduleBar(const std::string &processName, int startTime, int endTime, const std::string &state) {
    if (!series) return;
    
    QBarSet *set = new QBarSet(QString::fromStdString(processName));
    
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
    
    *set << (endTime - startTime);
    series->append(set);
}
