#pragma once

#include <optional>

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

    /** @brief get a thread local copy */
    virtual RC<VelocityField> cloneForParallelAccess() const = 0;
};
