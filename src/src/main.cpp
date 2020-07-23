#include <iostream>

#include <QApplication>

#include <crius/core/constantVelocityField.h>
#include <crius/core/fluentVelocityField.h>
#include <crius/ui/velocityMap.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    FluentVelocityField fvf("all-rke.cas");

    auto velField = newRC<ConstantVelocityField>(Vec3(1, 2, 3));

    VelocityMap map(nullptr, velField);
    map.resize(640, 480);
    map.show();

    app.exec();
}
