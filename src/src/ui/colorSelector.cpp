#include <QColorDialog>

#include <crius/ui/colorSelector.h>

ColorSelector::ColorSelector(const QColor &initColor, QWidget *parent)
    : QLabel(parent), isPressed_(false)
{
    setFixedHeight(height());
    setMinimumWidth(height());
    setScaledContents(true);
    setColor(initColor);
}

QColor ColorSelector::getColor() const noexcept
{
    return color_;
}

void ColorSelector::setColor(const QColor &color)
{
    color_ = color;
    updateLabel();
}

void ColorSelector::mousePressEvent(QMouseEvent *event)
{
    isPressed_ = true;
}

void ColorSelector::mouseReleaseEvent(QMouseEvent *event)
{
    if(!isPressed_)
        return;
    isPressed_ = false;

    const QColor newColor = QColorDialog::getColor(color_, this);
    if(newColor.isValid())
    {
        color_ = newColor;
        updateLabel();
        emit editColor();
    }
}

void ColorSelector::leaveEvent(QEvent *event)
{
    isPressed_ = false;
}

void ColorSelector::updateLabel()
{
    QImage img(1, 1, QImage::Format::Format_RGB888);
    img.setPixelColor(0, 0, color_);

    QPixmap pixmap;
    pixmap.convertFromImage(img);

    setPixmap(pixmap);
}
