#pragma once

#include <QComboBox>
#include <QMouseEvent>

#include <agz/utility/thread.h>

#include <crius/common/hsvColorMapper.h>
#include <crius/utility/doubleSlider.h>
#include <crius/velocityField/contour/velocityContourCache.h>

class VelocityContour;

/**
 * @brief QLabel which can emit a signal when resized
 */
class ContourRenderLabel : public QLabel
{
    Q_OBJECT

public:

    ContourRenderLabel(QWidget *parent, const VelocityContour *contour);

signals:

    void resize();

    void grabMove(int dX, int dY);

    void whellScroll(int x, int y, int dZ);

protected:

    void mousePressEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

    void leaveEvent(QEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void wheelEvent(QWheelEvent *event) override;

    void paintEvent(QPaintEvent *event) override;

    void resizeEvent(QResizeEvent *event) override;

private:

    void updateTooltop(int x, int y);

    const VelocityContour *contour_;

    int lastPressedX_ = 0;
    int lastPressedY_ = 0;
    bool middlePressed_ = false;

    QString positionText_;
    QString velocityText_;
};

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

    Vec3 toWorldPosition(int renderAreaX, int renderAreaY) const;

    std::optional<Vec3> getVelocity(const Vec3 &worldPos) const;

private:

    void getRenderAxis(
        int *horiAxis, int *vertAxis, int *depthAxis) const noexcept;

    void initializeWorldRect();

    void resizeWorldRect();

    void initializeDepth();

    void updateColorMapperVelRange();

    void render();

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

    ColorBar *colorBar_;
    ContourRenderLabel *renderArea_;

    bool isCacheDirty_ = true;
    VelocityContourCache contourCache_;
};
