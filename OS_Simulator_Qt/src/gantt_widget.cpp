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
    axisY->setRange(0, displayedAxisMax);
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

    // One bar set aligned with categories is more reliable for Qt bar rendering.
    auto *durations = new QBarSet("CPU Slice");
    durations->setColor(QColor(76, 175, 80));

    for (const auto &process : schedule) {
        const int duration = std::max(0, process.endTime - process.startTime);
        categories.append(QString::fromStdString(process.processName) +
                          QString(" [%1-%2]").arg(process.startTime).arg(process.endTime));
        *durations << duration;
        maxTime = std::max(maxTime, process.endTime);
    }
    
    axisX->setCategories(categories);
    animateAxisMax(maxTime + 5);

    series->append(durations);
}

void GanttWidget::clearChart() {
    if (series) {
        series->clear();
    }
    if (axisX) {
        axisX->clear();
    }
}

void GanttWidget::animateAxisMax(int targetMax) {
    if (!axisY) {
        return;
    }

    targetMax = std::max(5, targetMax);
    const int currentMax = static_cast<int>(axisY->max());
    displayedAxisMax = currentMax;

    if (targetMax == displayedAxisMax) {
        return;
    }

    if (axisAnimation) {
        axisAnimation->stop();
        axisAnimation->deleteLater();
        axisAnimation = nullptr;
    }

    axisAnimation = new QVariantAnimation(this);
    axisAnimation->setDuration(280);
    axisAnimation->setStartValue(displayedAxisMax);
    axisAnimation->setEndValue(targetMax);
    axisAnimation->setEasingCurve(QEasingCurve::OutCubic);

    connect(axisAnimation, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        axisY->setRange(0, value.toInt());
    });

    connect(axisAnimation, &QVariantAnimation::finished, this, [this, targetMax]() {
        displayedAxisMax = targetMax;
        if (axisAnimation) {
            axisAnimation->deleteLater();
            axisAnimation = nullptr;
        }
    });

    axisAnimation->start();
}
