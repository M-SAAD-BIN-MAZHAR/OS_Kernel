#include "philosophers_widget.h"
#include <QPainter>
#include <QtMath>

PhilosophersWidget::PhilosophersWidget(QWidget *parent)
    : QWidget(parent), states(5, "THINKING") {
    setMinimumHeight(170);
}

void PhilosophersWidget::setStates(const std::vector<QString> &newStates) {
    states = newStates;
    if (states.size() < 5) {
        states.resize(5, "THINKING");
    }
    update();
}

void PhilosophersWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor("#f8fafc"));

    const QPoint center(width() / 2, height() / 2);
    const int tableRadius = std::min(width(), height()) / 3;
    const int philosopherRadius = 26;
    const int forkRadius = tableRadius - 40;

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#d1d5db"));
    painter.drawEllipse(center, tableRadius - 30, tableRadius - 30);

    for (int i = 0; i < 5; ++i) {
        const double angle = (72.0 * i - 90.0) * M_PI / 180.0;
        const int px = center.x() + static_cast<int>(tableRadius * qCos(angle));
        const int py = center.y() + static_cast<int>(tableRadius * qSin(angle));

        painter.setBrush(colorForState(states[i]));
        painter.drawEllipse(QPoint(px, py), philosopherRadius, philosopherRadius);

        painter.setPen(Qt::black);
        painter.drawText(QRect(px - 22, py - 10, 44, 20), Qt::AlignCenter, QString("P%1").arg(i));
        painter.setPen(Qt::NoPen);

        const double forkAngle = (72.0 * i - 54.0) * M_PI / 180.0;
        const int fx = center.x() + static_cast<int>(forkRadius * qCos(forkAngle));
        const int fy = center.y() + static_cast<int>(forkRadius * qSin(forkAngle));
        painter.setBrush(QColor("#64748b"));
        painter.drawRect(fx - 4, fy - 12, 8, 24);
    }
}

QColor PhilosophersWidget::colorForState(const QString &state) const {
    if (state.contains("EAT", Qt::CaseInsensitive)) {
        return QColor("#16a34a");
    }
    if (state.contains("BLOCK", Qt::CaseInsensitive)) {
        return QColor("#dc2626");
    }
    if (state.contains("HUNGRY", Qt::CaseInsensitive)) {
        return QColor("#f59e0b");
    }
    return QColor("#0ea5e9");
}
