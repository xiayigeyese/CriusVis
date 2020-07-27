#pragma once

#include <QMainWindow>

#include <agz/utility/thread.h>

#include <crius/common.h>

class MainWindow : public QMainWindow
{
public:

    MainWindow();

private:

    void addFluentVelocityField();

    void addParticleDistribution();

    void addPathlines();

    QTabWidget *tabs_;

    RC<agz::thread::thread_group_t> threadGroup_;
};
