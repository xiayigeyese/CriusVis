
#include <QVBoxLayout>

#include <crius/velocityField/field3D/velocityField3D.h>

VelocityField3D::VelocityField3D(QWidget* parent, RC<const VelocityField> velocityField)
	:QWidget(parent), velocityField_(velocityField)
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
    arrowSizeSlider_ = new DoubleSlider(downPanel);
    arrowSize_->addItems({ "X - arrow width", "Y - arrow length", "Z - arrow thickness", "All" });
    arrowAxis_ = 3;
    arrowSize_->setCurrentIndex(arrowAxis_);
    arrowSizeSlider_->setRange(0, 5);
    arrowSizeSlider_->setValue(1);
    arrowSizeText->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    arrowSizeSlider_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

    // color maper & color bar
    colorMapper_ = new HSVColorMapper(downPanel);
    colorMapper_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    colorMapper_->hide();
    colorBar_ = new ColorBar(upPanel, colorMapper_);
    colorBar_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);
    
    // openglwidget
    openglWidget_ = new FieldRenderer(upPanel, velocityField_.get(), colorMapper_);
    openglWidget_->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    long sampleNum = openglWidget_->getSampleNums();

    // sample type
    auto velocitySampleText = new QLabel("Sample type", downPanel);
    velocitySampleType_ = new QComboBox(downPanel);
    velocitySampleType_->addItems({ "random sample", "uniform sample", "halton sequence sample" });
    velocitySampleType_->setCurrentIndex(0);
    velocitySampleText->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    
    // sample count
    auto velocityCountText = new QLabel(downPanel);
    auto velocityCountInput = new QSpinBox(downPanel);
    velocityCountText->setText("Sample count");
    velocityCountInput->setRange(1, 10000);
    velocityCountInput->setValue(sampleNum);
    velocityCountText->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    velocityCountInput->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    // bbox set bound x y z
    //xminSlider_ = new DoubleSlider(downPanel);
    //xmaxSlider_ = new DoubleSlider(downPanel);
    //yminSlider_ = new DoubleSlider(downPanel);
    //ymaxSlider_ = new DoubleSlider(downPanel);
    //zminSlider_ = new DoubleSlider(downPanel);
    //zmaxSlider_ = new DoubleSlider(downPanel);
    //auto xminSliderText_ = new QLabel("xmin", downPanel);
    //auto xmaxSliderText_ = new QLabel("xmax", downPanel);
    //auto yminSliderText_ = new QLabel("ymin", downPanel);
    //auto ymaxSliderText_ = new QLabel("ymax", downPanel);
    //auto zminSliderText_ = new QLabel("zmin", downPanel);
    //auto zmaxSliderText_ = new QLabel("zmax", downPanel);
    //xminSliderText_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    //xmaxSliderText_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    //yminSliderText_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    //ymaxSliderText_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    //zminSliderText_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    //zmaxSliderText_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    //AABB velocityBBox = velocityField_->getBoundingBox();
    //xminSlider_->setRange(velocityBBox.lower.x, velocityBBox.upper.x);
    //xminSlider_->setValue(velocityBBox.lower.x);
    //xmaxSlider_->setRange(velocityBBox.lower.x, velocityBBox.upper.x);
    //xmaxSlider_->setValue(velocityBBox.upper.x);
    //yminSlider_->setRange(velocityBBox.lower.y, velocityBBox.upper.y);
    //yminSlider_->setValue(velocityBBox.lower.y);
    //ymaxSlider_->setRange(velocityBBox.lower.y, velocityBBox.upper.y);
    //ymaxSlider_->setValue(velocityBBox.upper.y);
    //zminSlider_->setRange(velocityBBox.lower.z, velocityBBox.upper.z);
    //zminSlider_->setValue(velocityBBox.lower.z);
    //zmaxSlider_->setRange(velocityBBox.lower.z, velocityBBox.upper.z);
    //zmaxSlider_->setValue(velocityBBox.upper.z);

    upLayout->addWidget(colorBar_);
    upLayout->addWidget(openglWidget_);

    downLayout->addWidget(arrowSizeText, 0, 0, 1, 1);
    downLayout->addWidget(arrowSize_, 0, 1, 1, 1);
    downLayout->addWidget(arrowSizeSlider_, 1, 0, 1, 2);

    downLayout->addWidget(velocitySampleText, 2, 0, 1, 1);
    downLayout->addWidget(velocitySampleType_, 2, 1, 1, 1);
    downLayout->addWidget(velocityCountText, 3, 0, 1, 1);
    downLayout->addWidget(velocityCountInput, 3, 1, 1, 1);

    //downLayout->addWidget(xminSliderText_, 0, 2, 1, 1);
    //downLayout->addWidget(xminSlider_, 0, 3, 1, 1);
    //downLayout->addWidget(xmaxSlider_, 0, 4, 1, 1);
    //downLayout->addWidget(xmaxSliderText_, 0, 5, 1, 1);
    //
    //downLayout->addWidget(yminSliderText_, 1, 2, 1, 1);
    //downLayout->addWidget(yminSlider_, 1, 3, 1, 1);
    //downLayout->addWidget(ymaxSlider_, 1, 4, 1, 1);
    //downLayout->addWidget(ymaxSliderText_, 1, 5, 1, 1);
    //
    //downLayout->addWidget(zminSliderText_, 2, 2, 1, 1);
    //downLayout->addWidget(zminSlider_, 2, 3, 1, 1);
    //downLayout->addWidget(zmaxSlider_, 2, 4, 1, 1);
    //downLayout->addWidget(zmaxSliderText_, 2, 5, 1, 1);

    connect(colorMapper_, &VelocityColorMapper::editParams,
        [&]
        {
            colorBar_->redraw();
        });

    connect(arrowSize_, &QComboBox::currentTextChanged,
        [&](const QString&)
        {
            arrowAxis_ = arrowSize_->currentIndex();
            float value = openglWidget_->getArrowScale(arrowAxis_);
            arrowSizeSlider_->setValue(value);
        });

    connect(arrowSizeSlider_, &DoubleSlider::changeValue,
        [&]
        {
            float value = arrowSizeSlider_->getValue();
            if (arrowAxis_ == 0)
                openglWidget_->setArrowXSize(value);
            else if (arrowAxis_ == 1)
                openglWidget_->setArrowYSize(value);
            else if (arrowAxis_ == 2)
                openglWidget_->setArrowZSize(value);
            else
                openglWidget_->setArrowSize(value, value, value);
            openglWidget_->renderForReScaleArrow();
        });

    connect(velocityCountInput, qOverload<int>(&QSpinBox::valueChanged),
        [this](int newValue)
        {
            openglWidget_->setSampleNums(newValue);
            openglWidget_->renderForReSample();
        });

    connect(velocitySampleType_, &QComboBox::currentTextChanged,
        [&](const QString&)
        {
            int sampleType = velocitySampleType_->currentIndex();
            if(sampleType==0)
                openglWidget_->setSampleType(FieldRenderer::RANDOMSAMPLE);
            else if(sampleType==1)
                openglWidget_->setSampleType(FieldRenderer::UNIFORMSAMPLE);
            else if(sampleType==2)
                openglWidget_->setSampleType(FieldRenderer::HALTONSAMPLE);
            openglWidget_->renderForReSample();
        });

    //connect(xminSlider_, &DoubleSlider::changeValue,
    //    [&]
    //    {
    //       
    //    });
    //
    //connect(xmaxSlider_, &DoubleSlider::changeValue,
    //    [&]
    //    {
    //
    //    });
    //
    //connect(yminSlider_, &DoubleSlider::changeValue,
    //    [&]
    //    {
    //
    //    });
    //connect(ymaxSlider_, &DoubleSlider::changeValue,
    //    [&]
    //    {
    //
    //    });
    //
    //connect(zminSlider_, &DoubleSlider::changeValue,
    //    [&]
    //    {
    //
    //    });
    //connect(zmaxSlider_, &DoubleSlider::changeValue,
    //    [&]
    //    {
    //
    //    });
}
