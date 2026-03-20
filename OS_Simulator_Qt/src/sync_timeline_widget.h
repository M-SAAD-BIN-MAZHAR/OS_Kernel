#pragma once

#include <QWidget>
#include <QString>
#include <vector>

struct SyncTimelineSpan {
    QString actor;
    int startTick = 0;
    int endTick = 0;
    QString state;
};

class SyncTimelineWidget : public QWidget {
    Q_OBJECT

public:
    explicit SyncTimelineWidget(QWidget *parent = nullptr);
    void setTimeline(const std::vector<SyncTimelineSpan> &newTimeline);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QColor colorForState(const QString &state) const;

    std::vector<SyncTimelineSpan> timeline;
};
