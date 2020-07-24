#include <QApplication>
#include <QStyleFactory>

#include <crius/ui/mainWindow.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    qApp->setStyle(QStyleFactory::create("Fusion"));

    MainWindow main;
    main.showMaximized();

    app.exec();
}
