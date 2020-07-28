#pragma once

#include <QLabel>

#include <crius/common.h>

/**
 * @brief interface for map velocity to color
 */
class VelocityColorMapper : public QWidget
{
    Q_OBJECT

public:

    using QWidget::QWidget;

    virtual ~VelocityColorMapper() = default;

    virtual void setVelocityRange(float minVel, float maxVel) = 0;

    virtual QColor getColor(const Vec3 &velocity) const noexcept = 0;

    virtual QColor getColor(float velocity) const noexcept = 0;

signals:

    void editParams();
};

/**
 * @brief display a velocity-interpolated color bar
 */
class ColorBar : public QLabel
{
public:

    explicit ColorBar(QWidget *parent, VelocityColorMapper *colorMapper);

    void setParams(float lowVel, float highVel);

    void redraw();

protected:

    void paintEvent(QPaintEvent *event) override;

private:

    static constexpr int W = 128;
    static constexpr int BAR_X = 100;

    VelocityColorMapper *colorMapper_;
    float lowVel_, highVel_;
};
