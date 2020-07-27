#include <QGridLayout>
#include <QVBoxLayout>

#include <crius/particle/particleDistributionVisualizer.h>
#include <crius/utility/doubleSlider.h>

ParticleDistributionVisualizer::ParticleDistributionVisualizer(
    QWidget           *parent,
    const std::string &particleXMLFilename)
    : QWidget(parent)
{
    auto layout     = new QVBoxLayout(this);
    auto upPanel    = new QFrame(this);
    auto downPanel  = new QFrame(this);
    auto upLayout   = new QHBoxLayout(upPanel);
    auto downLayout = new QGridLayout(downPanel);

    upPanel->setFrameShape(QFrame::Box);
    downPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    layout->addWidget(upPanel);
    layout->addWidget(downPanel);

    ParticleLoader loader;
    loader.loadFromXML(particleXMLFilename);

    auto perspectiveCameraText = new QLabel("Perspective camera", downPanel);

    minVel_ = std::numeric_limits<float>::max(), maxVel_ = -minVel_;
    for(auto &p : loader.getAllParticles())
    {
        minVel_ = std::min(minVel_, p.colorBy);
        maxVel_ = std::max(maxVel_, p.colorBy);
    }
    maxVel_ = (std::max)(minVel_ + 0.01f, maxVel_);
    colorMapper_ = new HSVColorMapper(downPanel);
    colorMapper_->setVelocityRange(minVel_, maxVel_);
    colorMapper_->hide();

    colorBar_ = new ColorBar(upPanel, colorMapper_);
    colorBar_->setParams(minVel_, maxVel_);

    std::transform(
        loader.getAllParticles().begin(),
        loader.getAllParticles().end(),
        std::back_inserter(particles_),
        [&](const ParticleLoader::Particle &p)
    {
        const QColor color = colorMapper_->getColor(p.colorBy);
        colorBy_.push_back(p.colorBy);

        ParticleRenderer::Particle ret;
        ret.offset = p.position;
        ret.color  = {
            static_cast<float>(color.redF()),
            static_cast<float>(color.greenF()),
            static_cast<float>(color.blueF())
        };
        return ret;
    });

    renderer_          = new ParticleRenderer(upPanel, particles_);
    perspectiveCamera_ = new QCheckBox(downPanel);
    useDefaultCamera_  = new QPushButton("Use default camera", downPanel);

    auto renderCountText  = new QLabel(this);
    auto renderCountInput = new QSpinBox(downPanel);

    auto clipNearDistanceText = new QLabel(this);
    auto clipNearDistanceInput = new DoubleSlider(downPanel);

    auto clipFarDistanceText = new QLabel(this);
    auto clipFarDistanceInput = new DoubleSlider(downPanel);

    renderCountText->setText("Number of rendered particles: ");
    renderCountInput->setRange(1, static_cast<int>(particles_.size()));
    renderCountInput->setValue(static_cast<int>(particles_.size()));

    clipNearDistanceText->setText("   Near clipping distance: ");
    clipNearDistanceInput->setRange(0, 1);
    clipNearDistanceInput->setValue(0);

    clipFarDistanceText->setText("   Far clipping distance: ");
    clipFarDistanceInput->setRange(0, 1);
    clipFarDistanceInput->setValue(1);

    perspectiveCamera_->setChecked(false);

    perspectiveCameraText->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    perspectiveCamera_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    renderCountText->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    renderCountInput->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    clipNearDistanceText->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    clipNearDistanceInput->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    upLayout->addWidget(colorBar_);
    upLayout->addWidget(renderer_);

    downLayout->addWidget(perspectiveCameraText, 0, 0, 1, 1);
    downLayout->addWidget(perspectiveCamera_,    0, 1, 1, 1);
    downLayout->addWidget(useDefaultCamera_,     0, 2, 1, 1);
    downLayout->addWidget(renderCountText,       1, 0, 1, 1);
    downLayout->addWidget(renderCountInput,      1, 1, 1, 2);
    downLayout->addWidget(clipNearDistanceText,  0, 5, 1, 1);
    downLayout->addWidget(clipNearDistanceInput, 0, 6, 1, 1);
    downLayout->addWidget(clipFarDistanceText,   1, 5, 1, 1);
    downLayout->addWidget(clipFarDistanceInput,  1, 6, 1, 1);

    connect(perspectiveCamera_, &QCheckBox::stateChanged,
            [&](int)
    {
        renderer_->usePerspectiveCamera(perspectiveCamera_->isChecked());
    });

    connect(useDefaultCamera_, &QPushButton::clicked,
            [&](bool)
    {
        renderer_->useDefaultCamera();
    });

    connect(renderCountInput, qOverload<int>(&QSpinBox::valueChanged),
            [this](int newValue)
    {
        renderer_->setRenderedCount(newValue);
    });

    connect(clipNearDistanceInput, &DoubleSlider::changingValue,
        [this, clipNearDistanceInput]
    {
        renderer_->setNearClipDistance(
            static_cast<float>(clipNearDistanceInput->getValue()));
    });

    connect(clipFarDistanceInput, &DoubleSlider::changingValue,
        [this, clipFarDistanceInput]
    {
        renderer_->setFarClipDistance(
            static_cast<float>(clipFarDistanceInput->getValue()));
    });
}
