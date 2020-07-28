#pragma once

#include <QCheckBox>
#include <QPushButton>

#include <crius/pathline/pathlineRenderer.h>

class PathlineVisualizer : public QWidget
{
public:

    PathlineVisualizer(
        QWidget           *parent,
        const std::string &particleXMLFilename);

private:

    int pathlineCount_;

    PathlineRenderer *renderer_;
    QPushButton *useDefaultCamera_;
    QCheckBox *perspectiveCamera_;
};
