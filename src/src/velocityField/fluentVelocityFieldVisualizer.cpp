#include <crius/utility/closeEventDockWidget.h>
#include <crius/velocityField/fluentVelocityField.h>
#include <crius/velocityField/fluentVelocityFieldVisualizer.h>

VelocityFieldVisualizer::VelocityFieldVisualizer(
    QWidget *parent,
    const std::string &fluentCaseFilename,
    RC<agz::thread::thread_group_t> threadGroup)
    : QMainWindow(parent)
{
    filename_ = QString::fromStdString(fluentCaseFilename);

    menuBar()->addAction("Add Contour", [=] { addWindow(); });
    velocityField_ = newRC<FluentVelocityField>(fluentCaseFilename);

    threadGroup_.swap(threadGroup);

    addWindow();
}

void VelocityFieldVisualizer::addWindow()
{
    CloseEventDockWidget *dock = new CloseEventDockWidget(this);
    dock->setWindowTitle(QString());

    VelocityContour *contour = new VelocityContour(
        dock, velocityField_, threadGroup_);
    dock->setWidget(contour);

    dock->setAllowedAreas(Qt::AllDockWidgetAreas);
    addDockWidget(Qt::LeftDockWidgetArea, dock);

    dock->resize(640, 480);

    connect(dock, &CloseEventDockWidget::closeSignal, [=]
    {
        delete dock;
    });
}
