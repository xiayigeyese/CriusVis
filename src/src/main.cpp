#include <QApplication>
#include <QStyleFactory>

#include <crius/mainWindow.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    qApp->setStyle(QStyleFactory::create("Fusion"));

    MainWindow main;
    main.resize(640, 480);
    main.showMaximized();

    return QApplication::exec();
}
