#include <random>

#include <QMouseEvent>
#include <QOpenGLDebugLogger>
#include <QPainter>

#include <crius/pathline/pathlineRenderer.h>

namespace
{

    const char PATHLINE_VS[] = R"___(
    #version 330 core

    uniform mat4 projView;

    in vec3 position;
    in vec3 color;

    out vec3 o_color;

    void main()
    {
        o_color = color;
        gl_Position = projView * vec4(position, 1);
    }
    )___";

    const char PATHLINE_FS[] = R"___(
    #version 330 core

    in vec3 o_color;

    out vec4 fragColor;

    void main()
    {
        fragColor = vec4(o_color, 1);
    }
    )___";

    constexpr float PERSPECTIVE_FOV_RAD = agz::math::deg2rad(40.0f);

    constexpr float ORTHO_HEIGHT_OVER_DISTANCE = 0.72794f;

} // namespace anonymous

PathlineRenderer::PathlineRenderer(
    QWidget              *parent,
    std::vector<Pathline> pathlines)
    : QOpenGLWidget(parent), pathlines_(std::move(pathlines))
{
    // gl core profile version

    QSurfaceFormat format;
    format.setMajorVersion(3);
    format.setMinorVersion(3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setSamples(4);
#ifdef AGZ_DEBUG
    format.setOption(QSurfaceFormat::DebugContext);
#endif
    setFormat(format);

    // camera params

    boundingBox_ = { Vec3(0), Vec3(1) };
    useDefaultCamera();

    vertexCount_         = 0;
    vertexPerPathline_   = 0;
    renderedVertexCount_ = 0;
}

PathlineRenderer::~PathlineRenderer()
{
    makeCurrent();

    vertices_.destroy();
    vao_.destroy();

    doneCurrent();
}

void PathlineRenderer::initializeGL()
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

    // pathline shaders

    pathlineShader_.addShaderFromSourceCode(
        QOpenGLShader::Vertex, PATHLINE_VS);
    pathlineShader_.addShaderFromSourceCode(
        QOpenGLShader::Fragment, PATHLINE_FS);
    pathlineShader_.link();

    pathlineShader_.bindAttributeLocation("position", 0);
    pathlineShader_.bindAttributeLocation("color", 1);

    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

    setPathlines();
}

void PathlineRenderer::useDefaultCamera()
{
    horiRad_ = 0;
    vertRad_ = 0;
    distance_ = 1.1f * (boundingBox_.upper - boundingBox_.lower).length();
    lookAt_ = 0.5f * (boundingBox_.lower + boundingBox_.upper);
    update();
}

void PathlineRenderer::usePerspectiveCamera(bool perspective)
{
    perspective_ = perspective;
    update();
}

void PathlineRenderer::setRenderedCount(int renderedCount)
{
    renderedVertexCount_ = renderedCount * vertexPerPathline_;
    update();
}

void PathlineRenderer::setPathlines()
{
    // bounding box

    boundingBox_.lower = Vec3(std::numeric_limits<float>::max());
    boundingBox_.upper = Vec3(std::numeric_limits<float>::lowest());
    for(auto &p : pathlines_)
    {
        for(auto &pn : p.points)
        {
            boundingBox_.lower = elem_min(boundingBox_.lower, pn);
            boundingBox_.upper = elem_max(boundingBox_.upper, pn);
        }
    }

    // vertex data

    vertexPerPathline_   = 2 * static_cast<int>(pathlines_[0].points.size() - 1);
    vertexCount_         = static_cast<int>(pathlines_.size() * vertexPerPathline_);
    renderedVertexCount_ = vertexCount_;

    std::vector<Vertex> vertexData;
    vertexData.reserve(vertexCount_);

    std::default_random_engine rng{ std::random_device()() };
    const std::uniform_real_distribution<float> hueDis(0, 1);
    std::shuffle(pathlines_.begin(), pathlines_.end(), rng);

    for(auto &pathline : pathlines_)
    {
        const float hue = hueDis(rng);
        const QColor qcolor = QColor::fromHsvF(hue, 1, 1);
        const agz::math::color3f color = {
            static_cast<float>(qcolor.redF()),
            static_cast<float>(qcolor.greenF()),
            static_cast<float>(qcolor.blueF())
        };

        for(size_t i = 1; i < pathline.points.size(); ++i)
        {
            vertexData.push_back(
                { agz::math::vec3f(pathline.points[i - 1]), color });
            vertexData.push_back(
                { agz::math::vec3f(pathline.points[i]), color });
        }
    }

    assert(static_cast<int>(vertexData.size()) == vertexCount_);

    vertices_.destroy();
    vertices_.create();
    vertices_.bind();
    vertices_.allocate(
        vertexData.data(),
        static_cast<int>(sizeof(Vertex) * vertexData.size()));
    vertices_.release();

    // vao

    vao_.destroy();
    vao_.create();
    vao_.bind();

    vertices_.bind();
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 3, GL_FLOAT, false,
        sizeof(Vertex),
        reinterpret_cast<void *>(offsetof(Vertex, position)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, 3, GL_FLOAT, false,
        sizeof(Vertex),
        reinterpret_cast<void *>(offsetof(Vertex, color)));
    vertices_.release();

    vao_.release();

    pathlines_.clear();
    useDefaultCamera();
}

void PathlineRenderer::paintGL()
{
    glClearColor(0, 0.3f, 0.3f, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(renderedVertexCount_ <= 0)
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

    vao_.bind();
    pathlineShader_.bind();

    const QMatrix4x4 vp(&projView.data[0][0]);
    pathlineShader_.setUniformValue(
        pathlineShader_.uniformLocation("projView"), vp);

    glDrawArrays(GL_LINES, 0, renderedVertexCount_);

    pathlineShader_.release();
    vao_.release();
}

void PathlineRenderer::mousePressEvent(QMouseEvent *event)
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

void PathlineRenderer::mouseReleaseEvent(QMouseEvent *event)
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

void PathlineRenderer::mouseMoveEvent(QMouseEvent *event)
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

void PathlineRenderer::leaveEvent(QEvent *event)
{
    rightPressed_ = false;
    middlePressed_ = false;
}

void PathlineRenderer::wheelEvent(QWheelEvent *event)
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

    update();
}

void PathlineRenderer::paintEvent(QPaintEvent *event)
{
    QOpenGLWidget::paintEvent(event);

    QPainter painter(this);
    painter.setPen(Qt::white);
    QFontMetrics fm(painter.font());

    if(vertexPerPathline_ > 0)
    {
        painter.drawText(
            0, fm.height(),
            QString(" Number of pathlines          : %1").arg(
                vertexCount_ / vertexPerPathline_));

        painter.drawText(
            0, fm.height() + fm.height(),
            QString(" Number of rendered pathlines : %1").arg(
                renderedVertexCount_ / vertexPerPathline_));
    }
}
