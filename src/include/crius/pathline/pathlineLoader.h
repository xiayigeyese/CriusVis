#pragma once

#include <crius/common.h>

class PathlineLoader
{
public:

    struct Timepoint
    {
        float time;
        Vec3 position;
    };

    struct Pathline
    {
        std::vector<Timepoint> points;
    };

    void loadFromXML(const std::string &filename);

    const std::vector<Pathline> &getAllPathlines() const noexcept;

private:

    std::vector<Pathline> allPathlines_;
};
