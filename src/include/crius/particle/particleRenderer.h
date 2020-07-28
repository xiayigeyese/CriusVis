#pragma once

#include <QOpenGLBuffer>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWidget>

#include <crius/particle/particleLoader.h>

class ParticleRenderer
    : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
public:

    struct Particle
    {
        agz::math::vec3f   offset;
        agz::math::color3f color;
    };

    ParticleRenderer(
        QWidget              *parent,
        std::vector<Particle> particles);

    ~ParticleRenderer();

    void useDefaultCamera();

    void usePerspectiveCamera(bool perspective);

    void setRenderedCount(int renderedCount);

    void setNearClipDistance(float distance);

    void setFarClipDistance(float distance);

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

    void setParticles(int renderedCount);

    struct ParticleVertex
    {
        agz::math::vec3f position;
        agz::math::vec3f normal;
    };

    std::vector<Particle> particles_;

    QTimer *wheelTimer_    = nullptr;
    bool isWheelScrolling_ = false;

    int lastRightPressX_ = 0;
    int lastRightPressY_ = 0;
    bool rightPressed_   = false;

    int lastMiddlePressX_ = 0;
    int lastMiddlePressY_ = 0;
    bool middlePressed_   = false;

    AABB boundingBox_;

    bool perspective_ = false;
    float horiRad_  = 0;
    float vertRad_  = 0;
    float distance_ = 1;
    Vec3 lookAt_;

    float nearClipDistance_ = 0;
    float farClipDistance_ = 1;

    QOpenGLShaderProgram particleShader_;

    int vertexCount_   = 0;
    int instanceCount_ = 0;
    int renderedCount_ = 0;

    QOpenGLBuffer            particleVertices_;
    QOpenGLBuffer            particleInstanceData_;
    QOpenGLVertexArrayObject particleVAO_;
};
