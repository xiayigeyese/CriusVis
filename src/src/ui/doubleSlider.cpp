#include <crius/ui/doubleSlider.h>

DoubleSlider::DoubleSlider(QWidget *parent)
    : QWidget(parent)
{
    minText_ = new QLabel(this);
    maxText_ = new QLabel(this);
    valText_ = new QLabel(this);

    slider_ = new QSlider(this);
    slider_->setOrientation(Qt::Horizontal);
    slider_->setRange(0, MAX_INT);

    auto layout = new QHBoxLayout(this);
    layout->addWidget(minText_);
    layout->addWidget(maxText_);
    layout->addWidget(valText_);
    layout->addWidget(slider_);

    value_ = 0;
    setRange(0, 1);
    setValue(0.5);

    connect(slider_, &QSlider::valueChanged,
            [&](int)
    {
        value_ = minVal_ + (maxVal_ - minVal_) * slider_->value() / MAX_INT;
        updateText();
        emit changeValue();
    });
}

void DoubleSlider::setRange(double minVal, double maxVal)
{
    minVal_ = minVal;
    maxVal_ = maxVal;

    const double oldVal = value_;
    value_ = agz::math::clamp(value_, minVal_, maxVal_);

    if(value_ != oldVal)
    {
        slider_->blockSignals(true);
        slider_->setValue(
            static_cast<int>((value_ - minVal)
                           / (maxVal - minVal)
                           * MAX_INT));
        slider_->blockSignals(false);
    }

    updateText();
}

void DoubleSlider::setValue(double value)
{
    slider_->blockSignals(true);
    value_ = agz::math::clamp(value, minVal_, maxVal_);
    slider_->setValue(
            static_cast<int>((value_ - minVal_)
                           / (maxVal_ - minVal_)
                           * MAX_INT));
    slider_->blockSignals(false);

    updateText();
}

double DoubleSlider::getValue() const noexcept
{
    return value_;
}

void DoubleSlider::updateText()
{
    minText_->setText(QString::number(minVal_, 'e', 3));
    maxText_->setText(QString::number(maxVal_, 'e', 3));
    valText_->setText(QString::number(value_, 'e', 3));
}
