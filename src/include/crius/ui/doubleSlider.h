#pragma once

#include <agz/utility/math.h>

#include <QLabel>
#include <QSlider>
#include <QHBoxLayout>

/**
 * @brief 'double' value slider widget
 */
class DoubleSlider : public QWidget
{
    Q_OBJECT

public:

    explicit DoubleSlider(QWidget *parent);

    void setRange(double minVal, double maxVal);

    void setValue(double value);

    double getValue() const noexcept;

signals:

    void changeValue();

private:

    void updateText();

    static constexpr int MAX_INT = 1000;

    double minVal_, maxVal_;
    double value_;

    QLabel *minText_, *maxText_, *valText_;
    QSlider *slider_;
};
