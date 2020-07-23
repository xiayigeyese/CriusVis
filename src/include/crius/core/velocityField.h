#pragma once

#include <optional>
#include <string>

#include "../common.h"

/**
 * @brief abstraction of 3d velocity field
 */
class VelocityField
{
public:

    enum VelocityComponent
    {
        X   = 0,
        Y   = 1,
        Z   = 2,
        All = 3
    };

    virtual ~VelocityField() = default;

    /**
     * @brief map: world position -> velocity value
     *
     * returns nullopt when velocity at worldPos is undefined
     */
    virtual std::optional<Vec3> getVelocity(const Vec3 &pos) const noexcept = 0;

    /** @brief get maximum value of velocity */
    virtual float getMaxVelocity(VelocityComponent component) const noexcept = 0;

    /** @brief get minimum value of velocity */
    virtual float getMinVelocity(VelocityComponent component) const noexcept = 0;

    /** @brief get a bounding box of non-zero region */
    virtual AABB getBoundingBox() const noexcept = 0;
};

/**
 * @brief velocity field loaded from cas file exported by Fluent
 */
class FluentVelocityField : public VelocityField
{
public:

    void loadFromCasFile(const std::string &filename);

    std::optional<Vec3> getVelocity(const Vec3 &pos) const noexcept override;

    float getMaxVelocity(VelocityComponent component) const noexcept override;

    float getMinVelocity(VelocityComponent component) const noexcept override;

    AABB getBoundingBox() const noexcept override;

private:

    // TODO
};

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
