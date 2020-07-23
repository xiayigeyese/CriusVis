#include <iostream>

#include <QApplication>
#include <QVBoxLayout>

#include <crius/common.h>
#include <crius/ui/colorSelector.h>
#include <crius/ui/velocityMap.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    auto velField = newRC<ConstantVelocityField>(Vec3(1, 2, 3));

    VelocityMap map(nullptr, velField);
    map.resize(640, 480);
    map.show();

    app.exec();
}
