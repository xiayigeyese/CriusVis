
#include <QVBoxLayout>

#include <crius/luo/velocityField3D.h>

VelocityField3D::VelocityField3D(QWidget* parent)
	:QWidget(parent)
{
    auto layout = new QVBoxLayout(this);
    auto upPanel = new QFrame(this);
    auto downPanel = new QWidget(this);
    auto upLayout = new QHBoxLayout(upPanel);
    auto downLayout = new QGridLayout(downPanel);

    upPanel->setFrameShape(QFrame::Box);
    downPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    layout->addWidget(upPanel);
    layout->addWidget(downPanel);

    // arror size
    auto arrowSizeText = new QLabel("Arrow Size", downPanel);
    arrowSize_ = new QComboBox(downPanel);
    arrowSize_->addItems({ "X", "Y", "Z", "All" });
    arrowSize_->setCurrentIndex(3);
    arrowSizeText->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    downLayout->addWidget(arrowSizeText);
    downLayout->addWidget(arrowSize_);

    // color maper & color bar
    colorMapper_ = new HueColorMapper(downPanel);
    colorMapper_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    downLayout->addWidget(colorMapper_, 0, 2, 2, 1);

    colorBar_ = new HUEColorBar(upPanel, colorMapper_);
    colorBar_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);
    upLayout->addWidget(colorBar_);

    // openglwidget
    openglWidget_ = new OpenGLWidget(upPanel);
    openglWidget_->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    upLayout->addWidget(openglWidget_);

    // slider
    auto depthSliderText = new QLabel("Depth", downPanel);
    depthSlider_ = new DoubleSlider(downPanel);
    depthSliderText->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    depthSlider_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    downLayout->addWidget(depthSliderText, 2, 0, 1, 1);
    downLayout->addWidget(depthSlider_, 2, 1, 1, 2);

    connect(colorMapper_, &VelocityColorMapper::editParams,
        [&]
        {
            colorBar_->redraw();
        });
}
