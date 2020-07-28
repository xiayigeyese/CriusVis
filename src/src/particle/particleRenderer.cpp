#include <iostream>
#include <random>

#include <QMouseEvent>
#include <QOpenGLDebugLogger>
#include <QPainter>
#include <QTimer>

#include <agz/utility/mesh.h>

#include <crius/particle/particleRenderer.h>

namespace
{

    const char PARTICLE_VS[] = R"___(
    #version 330 core

    uniform mat4 projView;

    in vec3 position;
    in vec3 normal;
    in vec3 offset;
    in vec3 color;

    out vec3 w_normal;
    out vec3 w_position;
    out vec3 w_offset;
    out vec3 o_color;

    void main()
    {
        w_normal = normal;
        w_offset = offset;
        w_position = position + offset;
        o_color = color;
        gl_Position = projView * vec4(position + offset, 1);
    }
)___";

    const char PARTICLE_FS[] = R"___(
    #version 330 core

    uniform vec3 eyePosition;
    uniform vec3 cameraDir;
    uniform float nearClipDistance;
    uniform float farClipDistance;

    in vec3 w_normal;
    in vec3 w_position;
    in vec3 w_offset;
    in vec3 o_color;

    out vec4 frag_color;

    void main()
    {
        float dis = dot(w_offset - eyePosition, cameraDir);
        if(dis < nearClipDistance || dis > farClipDistance)
            discard;

        vec3 posToEye = normalize(eyePosition - w_position);
        float light_factor = min(
            0.2 + max(0, dot(posToEye, normalize(w_normal))), 1);
        frag_color = vec4(light_factor * o_color, 1);
    }
    )___";

    constexpr float PERSPECTIVE_FOV_RAD = agz::math::deg2rad(40.0f);

    constexpr float ORTHO_HEIGHT_OVER_DISTANCE = 0.72794f;

} // namespace anonymous

ParticleRenderer::ParticleRenderer(
    QWidget              *parent,
    std::vector<Particle> particles)
    : QOpenGLWidget(parent), particles_(std::move(particles))
{
    // gl core profile version

    QSurfaceFormat format;
    format.setMajorVersion(3);
    format.setMinorVersion(3);
    format.setProfile(QSurfaceFormat::CoreProfile);
#ifdef AGZ_DEBUG
    format.setOption(QSurfaceFormat::DebugContext);
#endif
    setFormat(format);

    // camera params

    boundingBox_ = { Vec3(0), Vec3(1) };
    useDefaultCamera();

    vertexCount_ = 0;
    instanceCount_ = 0;

    wheelTimer_ = new QTimer(this);
    wheelTimer_->setSingleShot(true);
    connect(wheelTimer_, &QTimer::timeout,
        [&]
    {
        if(isWheelScrolling_)
        {
            isWheelScrolling_ = false;
            update();
        }
    });
}

ParticleRenderer::~ParticleRenderer()
{
    makeCurrent();

    particleVertices_.destroy();
    particleInstanceData_.destroy();
    particleVAO_.destroy();

    doneCurrent();
}

void ParticleRenderer::initializeGL()
{
    // debugger

    AGZ_WHEN_DEBUG({
        QOpenGLDebugLogger * logger = new QOpenGLDebugLogger(this);
        connect(logger, &QOpenGLDebugLogger::messageLogged,
                [=](const QOpenGLDebugMessage &msg)
        {
            std::cerr << msg.message().toStdString();
        });
        logger->initialize();
        logger->startLogging();
    });

    // opengl funcs

    initializeOpenGLFunctions();

    // particle shaders

    particleShader_.addShaderFromSourceCode(
        QOpenGLShader::Vertex, PARTICLE_VS);
    particleShader_.addShaderFromSourceCode(
        QOpenGLShader::Fragment, PARTICLE_FS);
    particleShader_.link();

    particleShader_.bindAttributeLocation("position", 0);
    particleShader_.bindAttributeLocation("normal", 1);
    particleShader_.bindAttributeLocation("offset", 2);
    particleShader_.bindAttributeLocation("color", 3);

    // particle mesh

    const auto triangles = agz::mesh::load_from_file("./asset/particleMesh.obj");
    std::vector<ParticleVertex> vertices;
    vertices.reserve(triangles.size() * 3);
    for(auto &tri : triangles)
    {
        for(int i = 0; i < 3; ++i)
        {
            vertices.push_back(
                { 0.04f * tri.vertices[i].position, tri.vertices[i].normal });
        }
    }
    vertexCount_ = static_cast<int>(vertices.size());

    particleVertices_.create();
    particleVertices_.bind();
    particleVertices_.allocate(
        vertices.data(), sizeof(ParticleVertex) * vertexCount_);
    particleVertices_.release();

    // particle data

    setParticles(particles_.size());
}

void ParticleRenderer::useDefaultCamera()
{
    horiRad_ = agz::math::PI_f / 2;
    vertRad_ = 0;
    distance_ = 1.1f * (boundingBox_.upper - boundingBox_.lower).length();
    lookAt_ = 0.5f * (boundingBox_.lower + boundingBox_.upper);
    update();
}

void ParticleRenderer::usePerspectiveCamera(bool perspective)
{
    perspective_ = perspective;
    update();
}

void ParticleRenderer::setRenderedCount(int renderedCount)
{
    renderedCount_ = renderedCount;
    update();
}

void ParticleRenderer::setNearClipDistance(float distance)
{
    nearClipDistance_ = distance;
    update();
}

void ParticleRenderer::setFarClipDistance(float distance)
{
    farClipDistance_ = distance;
    update();
}

void ParticleRenderer::setParticles(int renderedCount)
{
    assert(0 < renderedCount && renderedCount <= particles_.size());

    boundingBox_.lower = Vec3(std::numeric_limits<float>::max());
    boundingBox_.upper = Vec3(std::numeric_limits<float>::lowest());
    for(auto &p : particles_)
    {
        boundingBox_.lower = elem_min(boundingBox_.lower, p.offset);
        boundingBox_.upper = elem_max(boundingBox_.upper, p.offset);
    }

    std::vector<Particle> instanceData = particles_;

    std::default_random_engine rng{ std::random_device()() };
    std::shuffle(std::begin(instanceData), std::end(instanceData), rng);

    instanceCount_ = static_cast<int>(particles_.size());
    renderedCount_ = renderedCount;

    particleInstanceData_.destroy();
    particleInstanceData_.create();
    particleInstanceData_.bind();
    particleInstanceData_.allocate(
        instanceData.data(),
        static_cast<int>(sizeof(Particle) * instanceData.size()));
    particleInstanceData_.release();

    particleVAO_.destroy();
    particleVAO_.create();
    particleVAO_.bind();

    particleVertices_.bind();
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 3, GL_FLOAT, false,
        sizeof(ParticleVertex),
        reinterpret_cast<void*>(offsetof(ParticleVertex, position)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, 3, GL_FLOAT, false,
        sizeof(ParticleVertex),
        reinterpret_cast<void*>(offsetof(ParticleVertex, normal)));
    particleVertices_.release();

    particleInstanceData_.bind();
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2, 3, GL_FLOAT, false,
        sizeof(Particle),
        reinterpret_cast<void *>(offsetof(Particle, offset)));
    glVertexAttribDivisor(2, 1);

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(
        3, 3, GL_FLOAT, false,
        sizeof(Particle),
        reinterpret_cast<void *>(offsetof(Particle, color)));
    glVertexAttribDivisor(3, 1);
    particleInstanceData_.release();

    particleVAO_.release();

    particles_.clear();
    useDefaultCamera();
}

void ParticleRenderer::paintGL()
{
    glClearColor(0, 0.3f, 0.3f, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    if(instanceCount_ <= 0)
        return;

    const float wOverH = static_cast<float>(width()) / height();

    const Vec3 dir = {
        std::cos(vertRad_) * std::cos(horiRad_),
        std::sin(vertRad_),
        std::cos(vertRad_) * std::sin(horiRad_)
    };

    const Vec3 eye = lookAt_ - dir * distance_;

    const Mat4 proj = perspective_ ?
        Mat4::left_transform::perspective(
            PERSPECTIVE_FOV_RAD, wOverH, 0.001f, 10.0f) :
        Mat4::left_transform::orthographics(
            ORTHO_HEIGHT_OVER_DISTANCE * distance_ * wOverH,
            ORTHO_HEIGHT_OVER_DISTANCE * distance_,
            0.001f, 10.0f);
    const Mat4 view = Mat4::left_transform::look_at(
        eye, lookAt_, Vec3(0, 1, 0));
    const Mat4 projView = (proj * view).transpose();

    particleVAO_.bind();
    particleShader_.bind();

    const QMatrix4x4 vp(&projView.data[0][0]);
    particleShader_.setUniformValue(
        particleShader_.uniformLocation("projView"), vp);
    particleShader_.setUniformValue(
        particleShader_.uniformLocation("eyePosition"),
        QVector3D(eye.x, eye.y, eye.z));
    particleShader_.setUniformValue(
        particleShader_.uniformLocation("cameraDir"),
        QVector3D(dir.x, dir.y, dir.z));

    const float diagLen = (boundingBox_.upper - boundingBox_.lower).length();
    const float centerDis = distance(
        0.5f * (boundingBox_.lower + boundingBox_.upper), eye);

    particleShader_.setUniformValue(
        particleShader_.uniformLocation("nearClipDistance"),
        agz::math::lerp(
            centerDis - 0.5f * diagLen,
            centerDis + 0.5f * diagLen,
            nearClipDistance_));

    particleShader_.setUniformValue(
        particleShader_.uniformLocation("farClipDistance"),
        agz::math::lerp(
            centerDis - 0.5f * diagLen,
            centerDis + 0.5f * diagLen,
            farClipDistance_));

    glDrawArraysInstanced(
        GL_TRIANGLES, 0, vertexCount_,
        middlePressed_ || rightPressed_ || isWheelScrolling_ ?
            (std::min)(renderedCount_, 100000) :
            renderedCount_);
    
    particleShader_.release();
    particleVAO_.release();
}

void ParticleRenderer::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::RightButton)
    {
        lastRightPressX_ = event->x();
        lastRightPressY_ = event->y();
        rightPressed_ = true;

        update();
    }
    else if(event->button() == Qt::MiddleButton)
    {
        lastMiddlePressX_ = event->x();
        lastMiddlePressY_ = event->y();
        middlePressed_ = true;

        update();
    }
}

void ParticleRenderer::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::RightButton)
    {
        rightPressed_ = false;
        update();
    }
    else if(event->button() == Qt::MiddleButton)
    {
        middlePressed_ = false;
        update();
    }
}

void ParticleRenderer::mouseMoveEvent(QMouseEvent *event)
{
    if(rightPressed_)
    {
        constexpr float PI = agz::math::PI_f;

        const int dx = event->x() - lastRightPressX_;
        const int dy = event->y() - lastRightPressY_;
        lastRightPressX_ = event->x();
        lastRightPressY_ = event->y();

        vertRad_ = agz::math::clamp(
            vertRad_ - 0.003f * dy, -PI / 2 + 0.01f, PI / 2 - 0.01f);
        horiRad_ -= 0.003f * dx;

        update();
    }
    else if(middlePressed_)
    {
        const int H = height();

        const int dx = event->x() - lastMiddlePressX_;
        const int dy = event->y() - lastMiddlePressY_;
        lastMiddlePressX_ = event->x();
        lastMiddlePressY_ = event->y();

        float pixelToWorldScale;
        if(perspective_)
        {
            pixelToWorldScale =
                distance_ * 2 * std::tan(PERSPECTIVE_FOV_RAD / 2) / H;
        }
        else
            pixelToWorldScale = distance_ * ORTHO_HEIGHT_OVER_DISTANCE / H;

        const float worldX = dx * pixelToWorldScale;
        const float worldY = dy * pixelToWorldScale;

        const Vec3 dir = {
            std::cos(vertRad_) * std::cos(horiRad_),
            std::sin(vertRad_),
            std::cos(vertRad_) * std::sin(horiRad_)
        };
        const Vec3 ex = cross(dir, Vec3(0, 1, 0)).normalize();
        const Vec3 ey = cross(ex, dir);

        lookAt_ += worldX * ex + worldY * ey;

        update();
    }
}

void ParticleRenderer::leaveEvent(QEvent *event)
{
    rightPressed_  = false;
    middlePressed_ = false;
}

void ParticleRenderer::wheelEvent(QWheelEvent *event)
{
    const int dZ = event->angleDelta().y();

    if(dZ > 0)
    {
        for(int i = 0; i < dZ; i += 120)
            distance_ *= 0.90909f;
    }
    else
    {
        for(int i = 0; i > dZ; i -= 120)
            distance_ *= 1.1f;
    }

    distance_ = (std::max)(
        distance_, 0.2f * distance(boundingBox_.upper, boundingBox_.lower));

    isWheelScrolling_ = true;
    wheelTimer_->start(200);
    update();
}

void ParticleRenderer::paintEvent(QPaintEvent *event)
{
    QOpenGLWidget::paintEvent(event);

    QPainter painter(this);
    painter.setPen(Qt::white);
    QFontMetrics fm(painter.font());

    painter.drawText(
        0, fm.height(),
        QString(" Number of particles         : %1").arg(instanceCount_));
    painter.drawText(
        0, fm.height() + fm.height(),
        QString(" Number of rendered particles: %1").arg(renderedCount_));
}
