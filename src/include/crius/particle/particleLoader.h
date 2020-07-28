#pragma once

#include <crius/common.h>

class ParticleLoader
{
public:

    struct Particle
    {
        Vec3 position;
        float colorBy;
    };

    void loadFromXML(const std::string &filename);

    const std::vector<Particle> &getAllParticles() const noexcept;

private:

    std::vector<Particle> allParticles_;
};
