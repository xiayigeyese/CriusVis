#pragma once

#include <QLabel>

class ColorSelector : public QLabel
{
    Q_OBJECT

public:

    ColorSelector(const QColor &initColor, QWidget *parent);

    QColor getColor() const noexcept;

    void setColor(const QColor &color);

signals:

    void editColor();

protected:

    void mousePressEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

    void leaveEvent(QEvent *event) override;

private:

    void updateLabel();

    QColor color_;

    bool isPressed_;
};
