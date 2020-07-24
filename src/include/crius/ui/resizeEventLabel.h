#pragma once

#include <QLabel>

/**
 * @brief QLabel which can emit a signal when resized
 */
class ResizeEventLabel : public QLabel
{
    Q_OBJECT

public:

    using QLabel::QLabel;

signals:

    void resize();

protected:

    void resizeEvent(QResizeEvent *event) override
    {
        QLabel::resizeEvent(event);
        emit resize();
    }
};
