#include "sync_timeline_widget.h"
#include <QMap>
#include <QPainter>

SyncTimelineWidget::SyncTimelineWidget(QWidget *parent)
    : QWidget(parent) {
    setMinimumHeight(140);
}

void SyncTimelineWidget::setTimeline(const std::vector<SyncTimelineSpan> &newTimeline) {
    timeline = newTimeline;
    update();
}

void SyncTimelineWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor("#f4f7f9"));

    if (timeline.empty()) {
        painter.setPen(Qt::darkGray);
        painter.drawText(rect(), Qt::AlignCenter, "Timeline updates appear here");
        return;
    }

    QMap<QString, int> rowByActor;
    int rowIndex = 0;
    int maxTick = 1;
    for (const SyncTimelineSpan &span : timeline) {
        if (!rowByActor.contains(span.actor)) {
            rowByActor[span.actor] = rowIndex++;
        }
        maxTick = std::max(maxTick, span.endTick);
    }

    const int leftPadding = 140;
    const int topPadding = 16;
    const int rowHeight = 28;
    const int barHeight = 16;
    const int usableWidth = std::max(1, width() - leftPadding - 16);

    for (auto it = rowByActor.constBegin(); it != rowByActor.constEnd(); ++it) {
        const int y = topPadding + it.value() * rowHeight;
        painter.setPen(QColor("#374151"));
        painter.drawText(8, y + 14, it.key());
    }

    for (const SyncTimelineSpan &span : timeline) {
        const int row = rowByActor.value(span.actor);
        const int y = topPadding + row * rowHeight;
        const int x1 = leftPadding + (span.startTick * usableWidth / maxTick);
        const int x2 = leftPadding + (span.endTick * usableWidth / maxTick);
        QRect barRect(x1, y, std::max(3, x2 - x1), barHeight);

        painter.setPen(Qt::NoPen);
        painter.setBrush(colorForState(span.state));
        painter.drawRoundedRect(barRect, 4, 4);
    }
}

QColor SyncTimelineWidget::colorForState(const QString &state) const {
    if (state.contains("BLOCK", Qt::CaseInsensitive)) {
        return QColor("#dc2626");
    }
    if (state.contains("EAT", Qt::CaseInsensitive) || state.contains("LOCK", Qt::CaseInsensitive)) {
        return QColor("#16a34a");
    }
    if (state.contains("MUTEX", Qt::CaseInsensitive)) {
        return QColor("#0284c7");
    }
    return QColor("#6b7280");
}
