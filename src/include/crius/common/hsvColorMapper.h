#pragma once

#include <QDoubleSpinBox>

#include <crius/common/velocityColorMapper.h>
#include <crius/utility/colorSelector.h>

/**
 * @brief map velocity to color with hsv interpolation
 */
class HSVColorMapper : public VelocityColorMapper
{
    Q_OBJECT

public:

    explicit HSVColorMapper(QWidget *parent);

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
