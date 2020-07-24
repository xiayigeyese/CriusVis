#pragma once

#include <QMenuBar>
#include <QMainWindow>

#include <crius/ui/velocityContour.h>

/**
 * @brief multi-view visualizer of a velocity field
 */
class VelocityFieldVisualizer : public QMainWindow
{
public:

    VelocityFieldVisualizer(
        QWidget                        *parent,
        const std::string              &fluentCaseFilename,
        RC<agz::thread::thread_group_t> threadGroup);

private:

    void addWindow();

    QString filename_;

    RC<agz::thread::thread_group_t> threadGroup_;
    RC<const VelocityField> velocityField_;
};
