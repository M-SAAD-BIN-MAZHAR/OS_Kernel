#pragma once

#include <QWidget>
#include <QString>
#include <vector>

class PhilosophersWidget : public QWidget {
    Q_OBJECT

public:
    explicit PhilosophersWidget(QWidget *parent = nullptr);
    void setStates(const std::vector<QString> &newStates);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QColor colorForState(const QString &state) const;

    std::vector<QString> states;
};
