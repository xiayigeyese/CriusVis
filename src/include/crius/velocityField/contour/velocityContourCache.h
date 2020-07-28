#pragma once

#include <agz/utility/texture.h>
#include <agz/utility/thread.h>

#include <crius/velocityField/velocityField.h>
#include <crius/common/velocityColorMapper.h>

/**
 * @brief velocity contour image cache
 */
class VelocityContourCache
{
public:

    static VelocityContourCache build(
        const VelocityColorMapper                  &colorMapper,
        int                                         resolution,
        int                                         threadCount,
        const std::vector<RC<const VelocityField>> &velocityFields,
        agz::thread::thread_group_t                &threadGroup,
        const Vec3i                                &axisIndices,
        float                                       depth,
        VelocityField::VelocityComponent            component);

    agz::math::color3b getValue(const Vec2 &worldPos) const noexcept;

private:

    Vec2 LB_, RT_;

    agz::texture::texture2d_t<agz::math::color3b> cache_;
};
