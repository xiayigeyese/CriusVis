#include <QApplication>
#include <QStyleFactory>
#include <QGuiApplication>

#include <crius/ui/mainWindow.h>
#include <crius/luo/openglWidget.h>
#include <crius/luo/velocityField3D.h>



int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    qApp->setStyle(QStyleFactory::create("Fusion"));

    MainWindow main;
    main.showMaximized();

    app.exec();

	return app.exec();
}
