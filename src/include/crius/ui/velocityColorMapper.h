#pragma once

#include <crius/common.h>
#include <crius/ui/colorSelector.h>

class VelocityColorMapper : public QWidget
{
    Q_OBJECT

public:

    explicit VelocityColorMapper(QWidget *parent);

    virtual ~VelocityColorMapper() = default;

    virtual void setVelocityRange(float minVel, float maxVel) = 0;

    virtual QColor getColor(const Vec3 &velocity) const noexcept = 0;

    virtual QColor getColor(float velocity) const noexcept = 0;

signals:

    void editParams();
};

class HSVColorMapper : public VelocityColorMapper
{
    Q_OBJECT

public:

    explicit HSVColorMapper(QWidget *parent);

    void setVelocityRange(float minVel, float maxVel) override;

    QColor getColor(const Vec3 &velocity) const noexcept override;

    QColor getColor(float velocity) const noexcept override;

private:

    float lowestVel_   = 0;
    float highestVel_  = 1;
    float rcpVelRange_ = 1;

    ColorSelector *highVelColor_;
    ColorSelector *lowVelColor_;
};
