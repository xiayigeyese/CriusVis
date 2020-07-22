#include <iostream>

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    std::cout << "hello, world" << std::endl;

    app.exec();
}
