#include <agz/utility/misc.h>
#include <agz/utility/texture.h>

#include <crius/ui/velocityContour.h>

VelocityContour::VelocityContour(
    QWidget                        *parent,
    RC<const VelocityField>         velocityField,
    RC<agz::thread::thread_group_t> threadGroup)
    : QWidget(parent)
{
    // layout

    auto layout     = new QVBoxLayout(this);
    auto upPanel    = new QFrame(this);
    auto downPanel  = new QWidget(this);
    auto upLayout   = new QHBoxLayout(upPanel);
    auto downLayout = new QGridLayout(downPanel);

    upPanel->setFrameShape(QFrame::Box);
    downPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    layout->addWidget(upPanel);
    layout->addWidget(downPanel);

    // render threads

    renderThreadCount_ = agz::thread::actual_worker_count(-1);
    renderThreadGroup_.swap(threadGroup);
    for(int i = 0; i < renderThreadCount_; ++i)
    {
        threadLocalVelocityField_.push_back(
            velocityField->cloneForParallelAccess());
    }

    // camera direction

    auto cameraDirectionText = new QLabel("Camera Direction", downPanel);
    cameraDirection_ = new QComboBox(downPanel);
    cameraDirection_->addItems({ "X", "Y", "Z" });
    cameraDirection_->setCurrentIndex(1);
    cameraDirectionText->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    downLayout->addWidget(cameraDirectionText, 0, 0, 1, 1);
    downLayout->addWidget(cameraDirection_, 0, 1, 1, 1);

    // velocity component

    auto velocityComponentText = new QLabel("Velocity Component", downPanel);
    velocityComponent_ = new QComboBox(downPanel);
    velocityComponent_->addItems({ "X", "Y", "Z" });
    velocityComponent_->setCurrentIndex(0);
    velocityComponentText->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    downLayout->addWidget(velocityComponentText, 1, 0, 1, 1);
    downLayout->addWidget(velocityComponent_, 1, 1, 1, 1);

    // depth slider

    auto depthSliderText = new QLabel("Depth", downPanel);
    depthSlider_ = new DoubleSlider(downPanel);
    depthSliderText->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    depthSlider_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    downLayout->addWidget(depthSliderText, 2, 0, 1, 1);
    downLayout->addWidget(depthSlider_, 2, 1, 1, 2);

    // color mapper & color bar

    colorMapper_ = new HueColorMapper(downPanel);
    colorMapper_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    downLayout->addWidget(colorMapper_, 0, 2, 2, 1);

    colorBar_ = new HUEColorBar(upPanel, colorMapper_);
    colorBar_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);
    upLayout->addWidget(colorBar_);

    // render area

    renderArea_ = new ResizeEventLabel(upPanel);
    renderArea_->setScaledContents(true);
    renderArea_->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    upLayout->addWidget(renderArea_);

    initializeWorldRect();
    initializeDepth();
    updateColorMapperVelRange();

    render();

    connect(cameraDirection_, &QComboBox::currentTextChanged,
            [&](const QString&)
    {
        initializeWorldRect();
        initializeDepth();
        render();
    });

    connect(velocityComponent_, &QComboBox::currentTextChanged,
            [&](const QString&)
    {
        updateColorMapperVelRange();
        render();
    });

    connect(depthSlider_, &DoubleSlider::changeValue,
            [&] { render(); });

    connect(colorMapper_, &VelocityColorMapper::editParams,
            [&]
    {
        colorBar_->redraw();
        render();
    });

    connect(renderArea_, &ResizeEventLabel::resize,
        [&]
    {
        resizeWorldRect();
        render();
    });
}

void VelocityContour::getRenderAxis(
    int *horiAxis, int *vertAxis, int *depthAxis) const noexcept
{
    switch(cameraDirection_->currentIndex())
    {
    case 0:
        if(horiAxis)  *horiAxis  = 1;
        if(vertAxis)  *vertAxis  = 2;
        if(depthAxis) *depthAxis = 0;
        break;
    case 1:
        if(horiAxis)  *horiAxis  = 0;
        if(vertAxis)  *vertAxis  = 2;
        if(depthAxis) *depthAxis = 1;
        break;
    default:
        if(horiAxis)  *horiAxis  = 0;
        if(vertAxis)  *vertAxis  = 1;
        if(depthAxis) *depthAxis = 2;
        break;
    }
}

void VelocityContour::initializeWorldRect()
{
    const int W = renderArea_->width(), H = renderArea_->height();

    const AABB aabb = threadLocalVelocityField_[0]->getBoundingBox();

    if(W <= 0 || H <= 0)
        return;

    const int xMarginPixels = W / 10;
    const int yMarginPixels = H / 10;

    int horiAxis, vertAxis;
    getRenderAxis(&horiAxis, &vertAxis, nullptr);

    const Vec2  l = Vec2(aabb.lower[horiAxis], aabb.lower[vertAxis]);
    const Vec2  u = Vec2(aabb.upper[horiAxis], aabb.upper[vertAxis]);
    const Vec2  m = Vec2(xMarginPixels, yMarginPixels);
    const Vec2  t = (Vec2(W, H) - m - m) / (u - l);
    const float s = (std::min)(t.x, t.y);
    const Vec2  o = 0.5f * (Vec2(W, H) - s * (l + u));

    leftBottomWorldPos_ = -o / s;
    rightTopWorldPos_   = (Vec2(W, H) - o) / s;

    oldW_ = W;
    oldH_ = H;
}

void VelocityContour::resizeWorldRect()
{
    initializeWorldRect();
}

void VelocityContour::initializeDepth()
{
    const auto aabb = threadLocalVelocityField_[0]->getBoundingBox();

    int depthAxis;
    getRenderAxis(nullptr, nullptr, &depthAxis);

    const float L = aabb.lower[depthAxis];
    const float U = aabb.upper[depthAxis];
    
    depthSlider_->setRange(L, U);
    depthSlider_->setValue(0.5f * (L + U));
}

void VelocityContour::updateColorMapperVelRange()
{
    const auto component = VelocityField::VelocityComponent(
        velocityComponent_->currentIndex());

    const float velL = threadLocalVelocityField_[0]->getMinVelocity(component);
    const float velU = threadLocalVelocityField_[0]->getMaxVelocity(component);

    colorMapper_->setVelocityRange(velL, velU);
    colorBar_->setParams(velL, velU);
}

void VelocityContour::render()
{
    const auto component = VelocityField::VelocityComponent(
        velocityComponent_->currentIndex());

    AGZ_DYN_FUNC_BOOL3_TEMPLATE(VelocityContour::renderImpl, renderFuncTable);

    auto renderFunc = renderFuncTable[
        agz::misc::bools_to_dyn_call_idx({
            component == VelocityField::X,
            component == VelocityField::Y,
            component == VelocityField::Z
            })];

    (this->*renderFunc)();
}

template<bool X, bool Y, bool Z>
void VelocityContour::renderImpl()
{
    const int W = renderArea_->width();
    const int H = renderArea_->height();

    const Vec2 a = (rightTopWorldPos_ - leftBottomWorldPos_) / Vec2(W, H);
    const Vec2 b = leftBottomWorldPos_;

    const auto pixelToWorld = [a, b](float x, float y)
    {
        return a * Vec2(x, y) + b;
    };

    agz::texture::texture2d_t<agz::math::color3b> imageData(H, W, agz::UNINIT);

    const float depth = depthSlider_->getValue();

    int horiAxis, vertAxis, depthAxis;
    getRenderAxis(&horiAxis, &vertAxis, &depthAxis);

    std::atomic<int> globalY = 0;
    renderThreadGroup_->run(
        renderThreadCount_, [&](int threadIndex)
    {
        auto &velocityField = threadLocalVelocityField_[threadIndex];

        for(;;)
        {
            const int y = globalY++;
            if(y >= H)
                return;

            for(int x = 0; x < W; ++x)
            {
                const Vec2 worldHV = pixelToWorld(x + 0.5f, y + 0.5f);

                Vec3 worldPos;
                worldPos[horiAxis] = worldHV.x;
                worldPos[vertAxis] = worldHV.y;
                worldPos[depthAxis] = depth;

                const auto vel = velocityField->getVelocity(worldPos);
                if(!vel)
                {
                    imageData(H - 1 - y, x) = { 255, 255, 255 };
                    continue;
                }

                QColor color;
                if constexpr(X)
                    color = colorMapper_->getColor(vel->x);
                else if constexpr(Y)
                    color = colorMapper_->getColor(vel->y);
                else
                    color = colorMapper_->getColor(vel->z);

                imageData(H - 1 - y, x) = to_color3b(agz::math::color3f(
                    color.redF(), color.greenF(), color.blueF()));
            }
        }
    });

    QImage image(
        &imageData.raw_data()->r, W, H,
        sizeof(agz::math::color3b) * W,
        QImage::Format_RGB888);

    QPixmap pixmap;
    pixmap.convertFromImage(image);
    renderArea_->setPixmap(pixmap);
}
