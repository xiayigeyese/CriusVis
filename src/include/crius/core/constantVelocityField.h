#pragma once

#include <crius/core/velocityField.h>

/**
 * @brief constant velocity field (for debugging)
 */
class ConstantVelocityField : public VelocityField
{
public:

    explicit ConstantVelocityField(const Vec3 &vel) noexcept
        : vel_(vel), len_(length(vel))
    {

    }

    std::optional<Vec3> getVelocity(const Vec3 &pos) const noexcept override
    {
        if(length(pos - Vec3(0.5f)) < 0.5f)
            return vel_;
        return std::nullopt;
    }

    float getMaxVelocity(VelocityComponent component) const noexcept override
    {
        if(component == All)
            return len_ + 0.1f;
        return vel_[component] + 0.1f;
    }

    float getMinVelocity(VelocityComponent component) const noexcept override
    {
        if(component == All)
            return len_ - 0.1f;
        return vel_[component] - 0.1f;
    }

    AABB getBoundingBox() const noexcept override
    {
        return { Vec3(0), Vec3(1) };
    }

private:

    Vec3 vel_;
    float len_;
};