#pragma once

#include <QComboBox>

#include <agz/utility/thread.h>

#include <crius/core/velocityField.h>
#include <crius/ui/velocityColorMapper.h>

class VelocityMap : public QWidget
{
    Q_OBJECT

public:

    VelocityMap(QWidget *parent, RC<const VelocityField> velocityField);

private slots:

    void onRepaint();

protected:

    void resizeEvent(QResizeEvent *event) override;

private:

    void updateColorMapperSettings();

    void drawMap(
        int horiAxis, int vertAxis, int depthAxis,
        VelocityField::VelocityComponent component);

    QLabel *renderArea_;
    QComboBox *viewAlong_;
    QComboBox *veloDir_;

    RC<const VelocityField> velocityField_;
    
    VelocityColorMapper *colorMapper_;

    agz::thread::thread_group_t threadGroup_;
};
