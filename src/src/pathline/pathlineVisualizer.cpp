#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>

#include <crius/pathline/pathlineVisualizer.h>

PathlineVisualizer::PathlineVisualizer(
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

    PathlineLoader loader;
    loader.loadFromXML(particleXMLFilename);
    std::vector<PathlineRenderer::Pathline> pathlines;
    for(auto &p : loader.getAllPathlines())
    {
        PathlineRenderer::Pathline pl;
        for(auto &pn : p.points)
            pl.points.push_back(pn.position);
        pathlines.push_back(pl);
    }
    pathlineCount_ = static_cast<int>(pathlines.size());

    auto perspectiveCameraText = new QLabel("Perspective camera", downPanel);

    renderer_          = new PathlineRenderer(upPanel, std::move(pathlines));
    perspectiveCamera_ = new QCheckBox(downPanel);
    useDefaultCamera_  = new QPushButton("Use default camera", downPanel);

    auto renderCountText  = new QLabel(this);
    auto renderCountInput = new QSpinBox(downPanel);

    renderCountText->setText("Number of rendered pathlines: ");
    renderCountInput->setRange(1, static_cast<int>(pathlineCount_));
    renderCountInput->setValue(static_cast<int>(pathlineCount_));

    perspectiveCamera_->setChecked(false);

    perspectiveCameraText->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    perspectiveCamera_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    renderCountText->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    renderCountInput->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    upLayout->addWidget(renderer_);

    downLayout->addWidget(perspectiveCameraText, 0, 0, 1, 1);
    downLayout->addWidget(perspectiveCamera_,    0, 1, 1, 1);
    downLayout->addWidget(useDefaultCamera_,     0, 2, 1, 1);
    downLayout->addWidget(renderCountText,       0, 3, 1, 1);
    downLayout->addWidget(renderCountInput,      0, 4, 1, 1);

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
}
