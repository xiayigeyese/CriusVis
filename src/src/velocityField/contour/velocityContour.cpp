#include <QPainter>

#include <agz/utility/misc.h>
#include <agz/utility/texture.h>

#include <crius/velocityField/contour/velocityContour.h>

ContourRenderLabel::ContourRenderLabel(
    QWidget *parent, const VelocityContour *contour)
    : QLabel(parent), contour_(contour)
{

}

void ContourRenderLabel::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::MiddleButton)
    {
        lastPressedX_ = event->x();
        lastPressedY_ = event->y();
        middlePressed_ = true;
    }
}

void ContourRenderLabel::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::MiddleButton)
        middlePressed_ = false;
}

void ContourRenderLabel::leaveEvent(QEvent *event)
{
    velocityText_ = QString("Velocity: nil");
    middlePressed_ = false;
}

void ContourRenderLabel::mouseMoveEvent(QMouseEvent *event)
{
    updateTooltop(event->x(), event->y());

    if(!middlePressed_)
        return;

    const int dX = event->x() - lastPressedX_;
    const int dY = event->y() - lastPressedY_;
    lastPressedX_ = event->x();
    lastPressedY_ = event->y();

    emit grabMove(dX, dY);
}

void ContourRenderLabel::wheelEvent(QWheelEvent *event)
{
    emit whellScroll(event->x(), event->y(), event->angleDelta().y());
}

void ContourRenderLabel::paintEvent(QPaintEvent *event)
{
    QLabel::paintEvent(event);

    QPainter painter(this);
    painter.setPen(Qt::white);

    QFontMetrics fm(painter.font());
    const int tightHeight = fm.tightBoundingRect(positionText_).height();
    const int height = fm.height();

    painter.drawText(0, tightHeight, positionText_);
    painter.drawText(0, tightHeight + height, velocityText_);
}

void ContourRenderLabel::resizeEvent(QResizeEvent *event)
{
    emit resize();
}

void ContourRenderLabel::updateTooltop(int x, int y)
{
    const Vec3 pos = contour_->toWorldPosition(x, y);
    const auto vel = contour_->getVelocity(pos);
    
    positionText_ = QString(" Position: (%1, %2, %3)")
                        .arg(pos.x).arg(pos.y).arg(pos.z)
                        .leftJustified(20);

    velocityText_ = vel ?
        QString(" Velocity: (%1, %2, %3)")
            .arg(vel->x).arg(vel->y).arg(vel->z) :
        QString(" Velocity: nil");

    update();
}

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

    colorMapper_ = new HSVColorMapper(downPanel);
    colorMapper_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    downLayout->addWidget(colorMapper_, 0, 2, 2, 1);

    colorBar_ = new ColorBar(upPanel, colorMapper_);
    colorBar_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);
    upLayout->addWidget(colorBar_);

    // render area

    renderArea_ = new ContourRenderLabel(upPanel, this);
    renderArea_->setScaledContents(true);
    renderArea_->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    renderArea_->setMouseTracking(true);
    upLayout->addWidget(renderArea_);

    initializeWorldRect();
    initializeDepth();
    updateColorMapperVelRange();

    render();

    connect(cameraDirection_, &QComboBox::currentTextChanged,
            [&](const QString&)
    {
        isCacheDirty_ = true;
        initializeWorldRect();
        initializeDepth();
        render();
    });

    connect(velocityComponent_, &QComboBox::currentTextChanged,
            [&](const QString&)
    {
        isCacheDirty_ = true;
        updateColorMapperVelRange();
        render();
    });

    connect(depthSlider_, &DoubleSlider::changeValue,
            [&]
    {
        isCacheDirty_ = true;
        render();
    });

    connect(colorMapper_, &VelocityColorMapper::editParams,
            [&]
    {
        isCacheDirty_ = true;
        colorBar_->redraw();
        render();
    });

    connect(renderArea_, &ContourRenderLabel::resize,
        [&]
    {
        resizeWorldRect();
        render();
    });

    connect(renderArea_, &ContourRenderLabel::grabMove,
            [&](int dX, int dY)
    {
        const int W = renderArea_->width();
        const int H = renderArea_->height();

        const Vec2 offset = (rightTopWorldPos_ - leftBottomWorldPos_)
                          * Vec2(-dX, dY) / Vec2(W, H);
        leftBottomWorldPos_ += offset;
        rightTopWorldPos_   += offset;

        render();
    });

    connect(renderArea_, &ContourRenderLabel::whellScroll,
            [&](int x, int y, int dZ)
    {
        const int W = renderArea_->width();
        const int H = renderArea_->height();

        const Vec2 a = (rightTopWorldPos_ - leftBottomWorldPos_) / Vec2(W, H);
        const Vec2 b = leftBottomWorldPos_;

        const Vec2 cursorWorld = a * Vec2(x + 0.5f, H - 0.5f - y) + b;
        Vec2 LBOffset = leftBottomWorldPos_ - cursorWorld;
        Vec2 RTOffset = rightTopWorldPos_   - cursorWorld;

        if(dZ > 0)
        {
            for(int i = 0; i < dZ; i += 120)
            {
                LBOffset *= 0.90909f;
                RTOffset *= 0.90909f;
            }
        }
        else
        {
            for(int i = 0; i > dZ; i -= 120)
            {

                LBOffset *= 1.1f;
                RTOffset *= 1.1f;
            }
        }

        leftBottomWorldPos_ = cursorWorld + LBOffset;
        rightTopWorldPos_   = cursorWorld + RTOffset;

        render();
    });
}

Vec3 VelocityContour::toWorldPosition(int renderAreaX, int renderAreaY) const
{
    const int W = renderArea_->width();
    const int H = renderArea_->height();
    if(!W || !H)
        return {};

    const Vec2 a = (rightTopWorldPos_ - leftBottomWorldPos_) / Vec2(W, H);
    const Vec2 b = leftBottomWorldPos_;
    const Vec2 worldXY = a * Vec2(renderAreaX, H - 1 - renderAreaY) + b;

    int horiAxis, vertAxis, depthAxis;
    getRenderAxis(&horiAxis, &vertAxis, &depthAxis);

    Vec3 worldPos;
    worldPos[horiAxis] = worldXY.x;
    worldPos[vertAxis] = worldXY.y;
    worldPos[depthAxis] = depthSlider_->getValue();

    return worldPos;
}

std::optional<Vec3> VelocityContour::getVelocity(const Vec3 &worldPos) const
{
    return threadLocalVelocityField_[0]->getVelocity(worldPos);
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
    if(W <= 0 || H <= 0)
        return;

    const AABB aabb = threadLocalVelocityField_[0]->getBoundingBox();

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
    const int W = renderArea_->width();
    const int H = renderArea_->height();
    if(!W || !H)
        return;

    const Vec2 worldCenter = 0.5f * (leftBottomWorldPos_ + rightTopWorldPos_);
    const Vec2 offset = worldCenter - leftBottomWorldPos_;
    const Vec2 scale = {
        static_cast<float>(W) / oldW_,
        static_cast<float>(H) / oldH_
    };
    Vec2 newOffset = scale * offset;
    newOffset.y = static_cast<float>(H) / W * newOffset.x;

    leftBottomWorldPos_ = worldCenter - newOffset;
    rightTopWorldPos_   = worldCenter + newOffset;

    oldW_ = W;
    oldH_ = H;
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

    if(isCacheDirty_)
    {
        isCacheDirty_ = false;

        contourCache_ = VelocityContourCache::build(
            *colorMapper_, 4096, renderThreadCount_,
            threadLocalVelocityField_, *renderThreadGroup_,
            { horiAxis, vertAxis, depthAxis }, depth, component);
    }

    std::atomic<int> globalY = 0;
    renderThreadGroup_->run(
        renderThreadCount_, [&](int threadIndex)
    {
        for(;;)
        {
            const int y = globalY++;
            if(y >= H)
                return;

            for(int x = 0; x < W; ++x)
            {
                const Vec2 worldHV = pixelToWorld(x + 0.5f, y + 0.5f);
                imageData(H - 1 - y, x) = contourCache_.getValue(worldHV);
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
