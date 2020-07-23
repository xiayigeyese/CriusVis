#include <QFormLayout>

#include <crius/ui/velocityColorMapper.h>

VelocityColorMapper::VelocityColorMapper(QWidget *parent)
    : QWidget(parent)
{
    
}

HSVColorMapper::HSVColorMapper(QWidget *parent)
    : VelocityColorMapper(parent)
{
    HSVColorMapper::setVelocityRange(0, 1);

    highVelColor_ = new ColorSelector(Qt::red, this);
    lowVelColor_  = new ColorSelector(Qt::blue, this);

    highVelColor_->setFixedWidth(5 * highVelColor_->height());
    lowVelColor_ ->setFixedWidth(5 * lowVelColor_ ->height());

    auto layout = new QFormLayout(this);
    layout->addRow("High Velocity", highVelColor_);
    layout->addRow("Low  Velocity", lowVelColor_);

    connect(highVelColor_, &ColorSelector::editColor,
            this, &HSVColorMapper::editParams);
    
    connect(lowVelColor_, &ColorSelector::editColor,
            this, &HSVColorMapper::editParams);

    layout->setContentsMargins(-1, 0, -1, 0);
    setContentsMargins(-1, 0, -1, 0);
}

void HSVColorMapper::setVelocityRange(float minVel, float maxVel)
{
    lowestVel_  = minVel;
    highestVel_ = (std::max)(maxVel, lowestVel_ + 0.001f);
    rcpVelRange_ = 1 / (maxVel - minVel);
}

QColor HSVColorMapper::getColor(const Vec3 &velocity) const noexcept
{
    return getColor(length(velocity));
}

QColor HSVColorMapper::getColor(float velocity) const noexcept
{
    const float t = glm::clamp(
        (velocity - lowestVel_) * rcpVelRange_, 0.0f, 1.0f);

    glm::dvec3 lowHSV, highHSV;
    lowVelColor_ ->getColor().getHsvF(&lowHSV.x, &lowHSV.y, &lowHSV.z);
    highVelColor_->getColor().getHsvF(&highHSV.x, &highHSV.y, &highHSV.z);
    const auto hsv = mix(lowHSV, highHSV, t);

    return QColor::fromHsvF(hsv.x, hsv.y, hsv.z);
}
