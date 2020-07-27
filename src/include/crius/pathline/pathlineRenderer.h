#pragma once

#include <QOpenGLBuffer>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWidget>

#include <crius/pathline/pathlineLoader.h>

class PathlineRenderer
    : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
public:

    struct Pathline
    {
        std::vector<agz::math::vec3f> points;
    };

    PathlineRenderer(
        QWidget              *parent,
        std::vector<Pathline> pathlines);

    ~PathlineRenderer();

    void useDefaultCamera();

    void usePerspectiveCamera(bool perspective);

    void setRenderedCount(int renderedCount);

protected:

    void initializeGL() override;

    void paintGL() override;

    void mousePressEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void leaveEvent(QEvent *event) override;

    void wheelEvent(QWheelEvent *event) override;

    void paintEvent(QPaintEvent *event) override;

private:

    void setPathlines();

    struct Vertex
    {
        agz::math::vec3f position;
        agz::math::color3f color;
    };

    std::vector<Pathline> pathlines_;

    int lastMiddlePressX_ = 0;
    int lastMiddlePressY_ = 0;
    bool middlePressed_   = false;

    int lastRightPressX_ = 0;
    int lastRightPressY_ = 0;
    bool rightPressed_   = false;

    AABB boundingBox_;

    bool perspective_ = false;
    float horiRad_  = 0;
    float vertRad_  = 0;
    float distance_ = 1;
    Vec3 lookAt_;

    QOpenGLShaderProgram pathlineShader_;

    int vertexCount_;
    int vertexPerPathline_;
    int renderedVertexCount_;

    QOpenGLBuffer            vertices_;
    QOpenGLVertexArrayObject vao_;
};
