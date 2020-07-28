#include <QFileDialog>
#include <QMessageBox>

#include <crius/particle/particleDistributionVisualizer.h>
#include <crius/pathline/pathlineVisualizer.h>
#include <crius/velocityField//fluentVelocityFieldVisualizer.h>

#include <crius/mainWindow.h>

MainWindow::MainWindow()
{
    threadGroup_ = newRC<agz::thread::thread_group_t>();

    tabs_ = new QTabWidget(this);
    tabs_->setTabsClosable(true);
    setCentralWidget(tabs_);

    menuBar()->addAction("Load Fluent Velocity Field", [&] { addFluentVelocityField(); });
    menuBar()->addAction("Load Particle Distribution", [&] { addParticleDistribution(); });
    menuBar()->addAction("Load Pathlines",             [&] { addPathlines(); });

    connect(tabs_, &QTabWidget::tabCloseRequested,
        [=](int index)
    {
        if(index < 0)
            return;

        auto item = tabs_->widget(index);
        tabs_->removeTab(index);

        delete item;
    });
}

void MainWindow::addFluentVelocityField()
{
    const auto filename = QFileDialog::getOpenFileName(
        this, "Load Fluent Cas File", QString(), "CAS (*.cas)");
    if(filename.isEmpty())
        return;

    try
    {
        auto vis = new VelocityFieldVisualizer(
            tabs_, filename.toStdString(), threadGroup_);

        tabs_->addTab(vis, QFileInfo(filename).fileName());
    }
    catch(const std::exception &e)
    {
        QMessageBox::information(this, "Error", e.what());
    }
}

void MainWindow::addParticleDistribution()
{
    const auto filename = QFileDialog::getOpenFileName(
        this, "Load Particle Distribution File", QString(), "XML (*.xml)");
    if(filename.isEmpty())
        return;

    try
    {
        auto vis = new ParticleDistributionVisualizer(
            tabs_, filename.toStdString());

        tabs_->addTab(vis, QFileInfo(filename).fileName());
    }
    catch(const std::exception &e)
    {
        QMessageBox::information(this, "Error", e.what());
    }
}

void MainWindow::addPathlines()
{
    const auto filename = QFileDialog::getOpenFileName(
        this, "Load Pathlines File", QString(), "XML (*.xml)");
    if(filename.isEmpty())
        return;

    try
    {
        auto vis = new PathlineVisualizer(
            tabs_, filename.toStdString());

        tabs_->addTab(vis, QFileInfo(filename).fileName());
    }
    catch(const std::exception &e)
    {
        QMessageBox::information(this, "Error", e.what());
    }
}
