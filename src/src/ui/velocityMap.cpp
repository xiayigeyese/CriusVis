#include <QGridLayout>

#include <agz/utility/texture.h>

#include <crius/ui/velocityMap.h>

VelocityMap::VelocityMap(QWidget *parent, RC<const VelocityField> velocityField)
    : QWidget(parent), velocityField_(std::move(velocityField))
{
    auto layout = new QVBoxLayout(this);
    auto downPanel = new QWidget(this);
    auto downLayout = new QGridLayout(downPanel);

    auto viewAlongText = new QLabel("View along ", downPanel);
    auto veloDirText   = new QLabel("Velocity.", downPanel);

    renderArea_  = new QLabel(this);
    viewAlong_   = new QComboBox(this);
    veloDir_     = new QComboBox(this);
    colorMapper_ = new HSVColorMapper(downPanel);

    renderArea_->setScaledContents(true);
    renderArea_->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    viewAlong_->addItems({ "X", "Y", "Z" });
    viewAlong_->setCurrentIndex(0);

    veloDir_->addItems({ "X", "Y", "Z", "All" });
    veloDir_->setCurrentIndex(3);

    downPanel    ->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    viewAlongText->setSizePolicy(QSizePolicy::Fixed,     QSizePolicy::Expanding);
    veloDirText  ->setSizePolicy(QSizePolicy::Fixed,     QSizePolicy::Expanding);
    colorMapper_ ->setSizePolicy(QSizePolicy::Fixed,     QSizePolicy::Expanding);
    
    layout->addWidget(renderArea_);
    layout->addWidget(downPanel);

    downLayout->addWidget(viewAlongText, 0, 0, 1, 1);
    downLayout->addWidget(veloDirText,   1, 0, 1, 1);
    downLayout->addWidget(viewAlong_,    0, 1, 1, 1);
    downLayout->addWidget(veloDir_,      1, 1, 1, 1);
    downLayout->addWidget(colorMapper_,  0, 2, 2, 1);

    connect(viewAlong_, &QComboBox::currentTextChanged,
            [&](const QString&) { onRepaint(); });
    connect(veloDir_, &QComboBox::currentTextChanged,
            [&](const QString&) { onRepaint(); });
    connect(colorMapper_, &VelocityColorMapper::editParams,
            this, &VelocityMap::onRepaint);

    onRepaint();
}

void VelocityMap::onRepaint()
{
    updateColorMapperSettings();

    const auto component = VelocityField::VelocityComponent(
        veloDir_->currentIndex());

    const auto viewAlong = viewAlong_->currentText();

    if(viewAlong == "X")
        drawMap(1, 2, 0, component);
    else if(viewAlong == "Y")
        drawMap(0, 2, 1, component);
    else
        drawMap(0, 1, 2, component);
}

void VelocityMap::resizeEvent(QResizeEvent *event)
{
    onRepaint();
}

void VelocityMap::updateColorMapperSettings()
{
    const auto veloDir = veloDir_->currentText();

    if(veloDir == "X")
    {
        colorMapper_->setVelocityRange(
            velocityField_->getMinVelocity(VelocityField::X),
            velocityField_->getMaxVelocity(VelocityField::X));
    }
    else if(veloDir == "Y")
    {
        colorMapper_->setVelocityRange(
            velocityField_->getMinVelocity(VelocityField::Y),
            velocityField_->getMaxVelocity(VelocityField::Y));
    }
    else if(veloDir == "Z")
    {
        colorMapper_->setVelocityRange(
            velocityField_->getMinVelocity(VelocityField::Z),
            velocityField_->getMaxVelocity(VelocityField::Z));
    }
    else if(veloDir == "All")
    {
        colorMapper_->setVelocityRange(
            velocityField_->getMinVelocity(VelocityField::All),
            velocityField_->getMaxVelocity(VelocityField::All));
    }
}

void VelocityMap::drawMap(
    int horiAxis, int vertAxis, int depthAxis,
    VelocityField::VelocityComponent component)
{
    const AABB aabb = velocityField_->getBoundingBox();
    
    const int horiPixels = renderArea_->width();
    const int vertPixels = renderArea_->height();

    if(horiPixels <= 0 || vertPixels <= 0)
        return;

    const int xMarginPixels = horiPixels / 10;
    const int yMarginPixels = vertPixels / 10;

    const Vec2  l = Vec2(aabb.lower[horiAxis], aabb.lower[vertAxis]);
    const Vec2  u = Vec2(aabb.upper[horiAxis], aabb.upper[vertAxis]);
    const Vec2  m = Vec2(xMarginPixels, yMarginPixels);
    const Vec2  t = (Vec2(horiPixels, vertPixels) - m - m) / (u - l);
    const float s = (std::min)(t.x, t.y);
    const Vec2  o = 0.5f * (Vec2(horiPixels, vertPixels) - s * (l + u));

    auto pixelToWorld = [&](int x, int y)
    {
        return (Vec2(x, y) - o) / s;
    };

    agz::texture::texture2d_t<agz::math::color3b> imageData(
        vertPixels, horiPixels, agz::UNINIT);

    std::atomic<int> globalY = 0;
    threadGroup_.run(-1, [&](int)
    {
        for(;;)
        {
            const int y = globalY++;
            if(y >= vertPixels)
                return;

            for(int x = 0; x < horiPixels; ++x)
            {
                const Vec2 worldHV = pixelToWorld(x, y);

                Vec3 worldPos;
                worldPos[horiAxis] = worldHV.x;
                worldPos[vertAxis] = worldHV.y;
                worldPos[depthAxis] = 0.5f * (aabb.lower[depthAxis]
                                            + aabb.upper[depthAxis]);

                const auto vel = velocityField_->getVelocity(worldPos);
                if(!vel)
                {
                    imageData(y, x) = { 255, 255, 255 };
                    continue;
                }

                QColor color;
                if(component == VelocityField::X)
                    color = colorMapper_->getColor(vel->x);
                else if(component == VelocityField::Y)
                    color = colorMapper_->getColor(vel->y);
                else if(component == VelocityField::Z)
                    color = colorMapper_->getColor(vel->z);
                else
                    color = colorMapper_->getColor(*vel);

                imageData(y, x) = to_color3b(agz::math::color3f(
                    color.redF(), color.greenF(), color.blueF()));
            }
        }
    });

    QImage image(
        &imageData.raw_data()->r, horiPixels, vertPixels,
        sizeof(agz::math::color3b) * horiPixels, QImage::Format_RGB888);

    QPixmap pixmap;
    pixmap.convertFromImage(image);
    renderArea_->setPixmap(pixmap);
}
