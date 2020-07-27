#pragma once

#include <QDockWidget>

/**
 * @brief QDockWidget which can emit a signal when close button is clicked
 */
class CloseEventDockWidget : public QDockWidget
{
    Q_OBJECT

public:

    using QDockWidget::QDockWidget;

signals:

    void closeSignal();

protected:

    void closeEvent(QCloseEvent *event) override
    {
        QDockWidget::closeEvent(event);
        emit closeSignal();
    }
};
