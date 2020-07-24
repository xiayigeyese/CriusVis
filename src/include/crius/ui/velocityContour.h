#pragma once

#include <QComboBox>

#include <agz/utility/thread.h>

#include <crius/core/velocityField.h>
#include <crius/ui/doubleSlider.h>
#include <crius/ui/hsvColorMapper.h>
#include <crius/ui/resizeEventLabel.h>

/**
 * @brief contour widget of a given velocity field
 */
class VelocityContour : public QWidget
{
    Q_OBJECT

public:

    VelocityContour(
        QWidget                        *parent,
        RC<const VelocityField>         velocityField,
        RC<agz::thread::thread_group_t> threadGroup);

private:

    void getRenderAxis(
        int *horiAxis, int *vertAxis, int *depthAxis) const noexcept;

    void initializeWorldRect();

    void resizeWorldRect();

    void initializeDepth();

    void updateColorMapperVelRange();

    void render();

    template<bool X, bool Y, bool Z>
    void renderImpl();

    int renderThreadCount_;
    RC<agz::thread::thread_group_t> renderThreadGroup_;
    std::vector<RC<const VelocityField>> threadLocalVelocityField_;

    QComboBox *cameraDirection_;
    QComboBox *velocityComponent_;

    Vec2 leftBottomWorldPos_;
    Vec2 rightTopWorldPos_;
    int oldW_ = 1;
    int oldH_ = 1;

    DoubleSlider *depthSlider_;

    VelocityColorMapper *colorMapper_;

    HUEColorBar *colorBar_;
    ResizeEventLabel *renderArea_;
};
