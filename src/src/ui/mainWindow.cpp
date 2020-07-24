#include <QFileDialog>
#include <QMessageBox>

#include <crius/ui/mainWindow.h>

MainWindow::MainWindow()
{
    threadGroup_ = newRC<agz::thread::thread_group_t>();

    tabs_ = new QTabWidget(this);
    tabs_->setTabsClosable(true);
    setCentralWidget(tabs_);

    menuBar()->addAction("Load Fluent Velocity Field", [&] { addFluentData(); });

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

void MainWindow::addFluentData()
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
