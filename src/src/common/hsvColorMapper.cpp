#include <QFormLayout>
#include <QPainter>

#include <agz/utility/texture.h>

#include <crius/common/hsvColorMapper.h>

ColorBar::ColorBar(QWidget *parent, VelocityColorMapper *colorMapper)
    : QLabel(parent), colorMapper_(colorMapper)
{
    lowVel_ = 0;
    highVel_ = 1;

    setFixedWidth(W);
    setScaledContents(true);

    redraw();
}

void ColorBar::setParams(
    float lowVel,
    float highVel)
{
    lowVel_    = lowVel;
    highVel_   = highVel;
    redraw();
}

void ColorBar::redraw()
{
    const int w = width();
    const int h = height();
    if(!w || !h)
        return;

    agz::texture::texture2d_t<agz::math::color3b> imgData(
        h, w, agz::math::color3b(255));

    for(int y = 0; y < h; ++y)
    {
        const float t = (y + 0.5f) / h;
        const float vel = agz::math::lerp(lowVel_, highVel_, t);
        const QColor color = colorMapper_->getColor(vel);

        const auto byteColor = to_color3b(agz::math::color3f(
            color.redF(), color.greenF(), color.blueF()));

        for(int x = BAR_X; x < w; ++x)
            imgData(h - 1 - y, x) = byteColor;
    }

    QImage image(
        &imgData.raw_data()->r, w, h, sizeof(agz::math::color3b) * w,
        QImage::Format_RGB888);

    QPixmap pixmap;
    pixmap.convertFromImage(image);

    setPixmap(pixmap);
}

void ColorBar::paintEvent(QPaintEvent *event)
{
    QLabel::paintEvent(event);

    const int w = width(), h = height();
    if(!w || !h)
        return;

    const int axisXEnd = BAR_X - 5;
    const int axisXBeg = axisXEnd - 10;

    QPainter painter(this);
    QFontMetrics fm(painter.font());

    painter.setBrush(QBrush(Qt::black));
    painter.drawLine(axisXEnd, 0, axisXEnd, height());

    const int hLineCnt = h / 30;
    for(int i = 0; i < hLineCnt; ++i)
    {
        const float t = i / (hLineCnt - 1.0f);
        const int y = static_cast<int>((1 - t) * (h - 1));

        painter.drawLine(axisXBeg, y, axisXEnd, y);

        const float vel = agz::math::lerp(lowVel_, highVel_, t);
        const QString velStr = QString::number(vel, 'e', 2);

        const int fontDy = fm.tightBoundingRect(velStr).height();

        if(i == 0)
            painter.drawText(0, y, velStr);
        else if(i == hLineCnt - 1)
            painter.drawText(0, y + fontDy, velStr);
        else
            painter.drawText(0, y + fontDy / 2, velStr);
    }
}

HSVColorMapper::HSVColorMapper(QWidget *parent)
    : VelocityColorMapper(parent)
{
    HSVColorMapper::setVelocityRange(0, 1);

    highVelColor_ = new ColorSelector(Qt::red, this);
    lowVelColor_  = new ColorSelector(Qt::blue, this);
    
    highVelColor_->setFixedWidth(3 * highVelColor_->height());
    lowVelColor_ ->setFixedWidth(3 * lowVelColor_ ->height());

    auto layout = new QFormLayout(this);
    layout->addRow("High Velocity", highVelColor_);
    layout->addRow("Low  Velocity", lowVelColor_);
    
    connect(highVelColor_, &ColorSelector::editColor,
            [&] { emit editParams(); });

    connect(lowVelColor_, &ColorSelector::editColor,
            [&] { emit editParams(); });

    layout->setContentsMargins(-1, 0, -1, 0);
    setContentsMargins(-1, 0, -1, 0);
}

void HSVColorMapper::setVelocityRange(float minVel, float maxVel)
{
    lowestVel_  = minVel;
    highestVel_ = (std::max)(maxVel, lowestVel_ + 0.001f);
    rcpVelRange_ = 1 / (maxVel - minVel);
}

QColor HSVColorMapper::getColor(const Vec3 &velocity) const noexcept
{
    return getColor(velocity.length());
}

QColor HSVColorMapper::getColor(float velocity) const noexcept
{
    const float t = agz::math::saturate(
        (velocity - lowestVel_) * rcpVelRange_);

    double low[3], high[3];
    lowVelColor_ ->getColor().getHsvF(&low[0], &low[1], &low[2]);
    highVelColor_->getColor().getHsvF(&high[0], &high[1], &high[2]);
    
    return QColor::fromHsvF(
        agz::math::saturate(agz::math::lerp(low[0], high[0], t)),
        agz::math::saturate(agz::math::lerp(low[1], high[1], t)),
        agz::math::saturate(agz::math::lerp(low[2], high[2], t)));
}
