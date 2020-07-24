#pragma once

#include <QDoubleSpinBox>
#include <QLabel>

#include <crius/ui/colorSelector.h>
#include <crius/ui/velocityColorMapper.h>

/**
 * @brief display a hsv interpolated color bar
 */
class HUEColorBar : public QLabel
{
public:

    explicit HUEColorBar(QWidget *parent, VelocityColorMapper *colorMapper);

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

/**
 * @brief map velocity to color with hsv interpolation
 */
class HueColorMapper : public VelocityColorMapper
{
    Q_OBJECT

public:

    explicit HueColorMapper(QWidget *parent);

    void setVelocityRange(float minVel, float maxVel) override;

    QColor getColor(const Vec3 &velocity) const noexcept override;

    QColor getColor(float velocity) const noexcept override;

private:

    float lowestVel_ = 0;
    float highestVel_ = 1;
    float rcpVelRange_ = 1;

    ColorSelector *highVelColor_;
    ColorSelector *lowVelColor_;
};
