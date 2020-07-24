#pragma once

#include <crius/ui/fluentVelocityFieldVisualizer.h>

class MainWindow : public QMainWindow
{
public:

    MainWindow();

private:

    void addFluentData();

    QTabWidget *tabs_;

    RC<agz::thread::thread_group_t> threadGroup_;
};
