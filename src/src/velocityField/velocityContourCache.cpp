#include <crius/velocityField/velocityContourCache.h>

VelocityContourCache VelocityContourCache::build(
    const VelocityColorMapper                  &colorMapper,
    int                                         resolution,
    int                                         threadCount,
    const std::vector<RC<const VelocityField>> &velocityFields,
    agz::thread::thread_group_t                &threadGroup,
    const Vec3i                                &axisIndices,
    float                                       depth,
    VelocityField::VelocityComponent            component)
{
    VelocityContourCache ret;
    ret.cache_.initialize(resolution, resolution, { 0, 77, 77 });

    const auto aabb = velocityFields[0]->getBoundingBox();
    ret.LB_.x = aabb.lower[axisIndices.x];
    ret.LB_.y = aabb.lower[axisIndices.y];
    ret.RT_.x = aabb.upper[axisIndices.x];
    ret.RT_.y = aabb.upper[axisIndices.y];

    const Vec2 a = (ret.RT_ - ret.LB_) / Vec2(resolution);
    const Vec2 b = ret.LB_;

    const auto pixelToWorld = [a, b](float x, float y)
    {
        return a * Vec2(x, y) + b;
    };

    std::atomic<int> globalY = 0;
    threadGroup.run(
        threadCount, [&](int threadIndex)
    {
        auto &velocityField = *velocityFields[threadIndex];

        for(;;)
        {
            const int y = globalY++;
            if(y >= resolution)
                return;

            Vec3 worldPos;
            worldPos[axisIndices.z] = depth;

            for(int x = 0; x < resolution; ++x)
            {
                const Vec2 worldXY = pixelToWorld(x, y);

                worldPos[axisIndices.x] = worldXY.x;
                worldPos[axisIndices.y] = worldXY.y;

                const auto vel = velocityField.getVelocity(worldPos);
                if(!vel)
                    continue;

                QColor color;
                if(component == VelocityField::X)
                    color = colorMapper.getColor(vel->x);
                else if(component == VelocityField::Y)
                    color = colorMapper.getColor(vel->y);
                else
                    color = colorMapper.getColor(vel->z);

                ret.cache_(y, x) = to_color3b(agz::math::color3f(
                    color.redF(), color.greenF(), color.blueF()));
            }
        }
    });

    return ret;
}

agz::math::color3b VelocityContourCache::getValue(const Vec2 &worldPos) const noexcept
{
    if(worldPos.x < LB_.x || worldPos.y < LB_.y ||
       worldPos.x > RT_.x || worldPos.y > RT_.y)
        return { 0, 77, 77 };

    const Vec2 uv = (worldPos - LB_) / (RT_ - LB_);

    return agz::texture::nearest_sample2d(
        uv, [&](int x, int y) { return cache_(y, x); },
        cache_.width(), cache_.height());
}
