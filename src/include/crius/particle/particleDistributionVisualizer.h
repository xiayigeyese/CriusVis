#pragma once

#include <QCheckBox>
#include <QPushButton>

#include <crius/common/hsvColorMapper.h>
#include <crius/particle/particleRenderer.h>

class ParticleDistributionVisualizer : public QWidget
{
public:

    ParticleDistributionVisualizer(
        QWidget           *parent,
        const std::string &particleXMLFilename);

private:

    ColorBar *colorBar_;
    VelocityColorMapper *colorMapper_;

    float minVel_, maxVel_;
    std::vector<ParticleRenderer::Particle> particles_;
    std::vector<float> colorBy_;

    ParticleRenderer *renderer_;
    QPushButton *useDefaultCamera_;
    QCheckBox *perspectiveCamera_;
};
