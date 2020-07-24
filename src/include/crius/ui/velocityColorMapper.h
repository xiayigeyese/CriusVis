#pragma once

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
