#include <crius/utility/closeEventDockWidget.h>
#include <crius/velocityField/field3D/velocityField3D.h>
#include <crius/velocityField/fluentVelocityField.h>
#include <crius/velocityField/fluentVelocityFieldVisualizer.h>

VelocityFieldVisualizer::VelocityFieldVisualizer(
    QWidget *parent,
    const std::string &fluentCaseFilename,
    RC<agz::thread::thread_group_t> threadGroup)
    : QMainWindow(parent)
{
    filename_ = QString::fromStdString(fluentCaseFilename);

    menuBar()->addAction("Add Contour", [=] { addContourWindow(); });
    menuBar()->addAction("Add Field3D", [=] { add3DWindow(); });
    velocityField_ = newRC<FluentVelocityField>(fluentCaseFilename);

    threadGroup_.swap(threadGroup);

    addContourWindow();
    add3DWindow();
}

void VelocityFieldVisualizer::addContourWindow()
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

void VelocityFieldVisualizer::add3DWindow()
{
    CloseEventDockWidget* dock = new CloseEventDockWidget(this);
    dock->setWindowTitle(QString());

    auto field3D = new VelocityField3D(dock, velocityField_);
    dock->setWidget(field3D);

    dock->setAllowedAreas(Qt::AllDockWidgetAreas);
    addDockWidget(Qt::RightDockWidgetArea, dock);

    dock->resize(640, 480);

    connect(dock, &CloseEventDockWidget::closeSignal, [=]
        {
            delete dock;
        });
}
