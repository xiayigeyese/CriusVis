#include <string>
#include <random>

#include <QSurfaceFormat>
#include <QOpenGLShaderProgram>
#include <QMouseEvent>
#include <QPoint>
#include <QMatrix4x4>
#include <QVector4D>
#include <QVector>
#include <QVBoxLayout>

#include <agz/utility/system.h>

#include <crius/velocityField/field3D/fieldRenderer.h>
#include <crius/velocityField/velocityField.h>

FieldRenderer::FieldRenderer(QWidget* parent, const VelocityField* velocityField, HSVColorMapper* colorMapper)
	: QOpenGLWidget(parent), velocityField_(velocityField), colorMapper_(colorMapper)
{
	QSurfaceFormat format;
	format.setMajorVersion(3);
	format.setMinorVersion(3);
	format.setProfile(QSurfaceFormat::CoreProfile);
	AGZ_WHEN_DEBUG({
		format.setOption(QSurfaceFormat::DebugContext);
		});
	setFormat(format);

	init();
}

FieldRenderer::~FieldRenderer()
{
	makeCurrent();
	glDeleteVertexArrays(1, &vao_);
	glDeleteBuffers(1, &vbo_);
	glDeleteBuffers(1, &instanceVBO_);
	delete shaderProgram_;
	doneCurrent();
}

void FieldRenderer::initializeGL()
{
	initializeOpenGLFunctions();

	// 创建shader
	if (!createShader()) 
		return;
	
	// 绑定数据
	glGenVertexArrays(1, &vao_);
	glGenBuffers(1, &vbo_);
	glBindVertexArray(vao_);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_);
	glBufferData(GL_ARRAY_BUFFER, arrowVertices_.size() * sizeof(Vec3), arrowVertices_.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3), (void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	bindInstanceVBOForPaint();

	glEnable(GL_DEPTH_TEST);
}

void FieldRenderer::paintGL()
{
	glClearColor(0, 0.3f, 0.3f, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	shaderProgram_->bind();
	eye_ = getCameraPosition();
	QMatrix4x4 view, projection;
	view.lookAt(eye_, lookAt_, QVector3D(0.0f, 1.0f, 0.0f));
	projection.perspective(fov_, 1.0f * width() / height(), 0.1f, 100.0f);
	shaderProgram_->setUniformValue("view", view);
	shaderProgram_->setUniformValue("projection", projection);

	glBindVertexArray(vao_);
	glDrawArraysInstanced(GL_TRIANGLES, 0, arrowVertices_.size(), sampleNum_);

	glBindVertexArray(0);
	shaderProgram_->release();
}

void FieldRenderer::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
		leftBtnPressed_ = true;
	else if (event->button() == Qt::RightButton)
		rightBtnPressed_ = true;
	else if (event->button() == Qt::MiddleButton)
		middleBtnPressed_ = true;
	lastPos_ = event->pos();
}

void FieldRenderer::mouseReleaseEvent(QMouseEvent* event)
{
	Q_UNUSED(event);
	leftBtnPressed_ = false;
	rightBtnPressed_ = false;
}

void FieldRenderer::mouseMoveEvent(QMouseEvent* event)
{
	QPoint nowPos = event->pos();
	int xoffset = nowPos.x() - lastPos_.x();
	int yoffset = lastPos_.y() - nowPos.y();
	lastPos_ = nowPos;
	if (rightBtnPressed_)
	{
		yaw_ += xoffset * 0.05;
		pitch_ = agz::math::clamp(pitch_ + yoffset * 0.05f, -89.5f, 89.5f);
	}
	else if (middleBtnPressed_)
	{
		QVector3D front = lookAt_ - eye_;
		QVector3D worldUp(0.0, 1.0, 0.0);
		QVector3D right = QVector3D::crossProduct(front, worldUp);
		QVector3D up = QVector3D::crossProduct(right, front);
		float h = cameraDistance_ * tanf(radians(fov_) / 2.0f);
		float dy =  1.0f * yoffset / height() * h * 2;
		float dx = 1.0f * xoffset / height() * h * 2;
		lookAt_ -= right.normalized() * dx + up.normalized() * dy ;
	}
	update();
}

void FieldRenderer::wheelEvent(QWheelEvent* event)
{
	QPoint offset = event->angleDelta();
	cameraDistance_ *= - offset.y() / 2400.0f + 1;
	cameraDistance_ = agz::math::clamp(cameraDistance_, 0.2f, 99.0f);
	update();
}

void FieldRenderer::setVelcotityField(VelocityField* velocityField)
{
	velocityField_ = velocityField;
	init();
}

void FieldRenderer::init() {
	// 初始化参数
	sampleNum_ = 1000;
	leftBtnPressed_ = false;
	rightBtnPressed_ = false;
	middleBtnPressed_ = false;

	// 加载箭头模型
	loadArrowMesh();
	
	// 速度场 获取最大速度和最小速度
	float maxVelocity = velocityField_->getMaxVelocity(FluentVelocityField::All);
	float minVelocity = velocityField_->getMinVelocity(FluentVelocityField::All);

	// 创建velocity到color的 colorMapper对象
	// colorMapper_ = new HSVColorMapper(this);
	colorMapper_->setVelocityRange(minVelocity, maxVelocity);

	// 获取速度场包围盒 和 速度场中心
	velocityFieldBBox_ = velocityField_->getBoundingBox();
	velocityFiledCenter_[0] = (velocityFieldBBox_.lower.x + velocityFieldBBox_.upper.x) / 2;
	velocityFiledCenter_[1] = (velocityFieldBBox_.lower.y + velocityFieldBBox_.upper.y) / 2;
	velocityFiledCenter_[2] = (velocityFieldBBox_.lower.z + velocityFieldBBox_.upper.z) / 2;

	// 根据包围盒 设置箭头大小
	scale_ = Vec3(1) * getArrowSize(velocityFieldBBox_, arrowBBox_, sampleNum_);
	scaleResize_ = Vec3(1);

	// 根据包围盒和采样数量采样
	sampleType_ = HALTONSAMPLE;
	samplePoints(velocityFieldBBox_, sampleNum_);

	// 获取instanceData
	constructInstanceData(colorMapper_, scale_ * scaleResize_);

	// 设置摄像机参数
	yaw_ = -90;
	pitch_ = 0;
	fov_ = 45.0f;
	cameraDistance_ = agz::math::distance(velocityFieldBBox_.lower, velocityFieldBBox_.upper) * 0.8;
	lookAt_ = velocityFiledCenter_;
}

void FieldRenderer::loadArrowMesh()
{
	std::string objFile = "asset/arrow.obj";
	auto triangles = agz::mesh::load_from_file(objFile);
	Vec3 lower = Vec3(std::numeric_limits<float>::max());
	Vec3 upper = Vec3(-std::numeric_limits<float>::max());
	for (auto& triangle : triangles)
	{
		for (int i = 0; i < 3; i++)
		{
			Vec3 position = triangle.vertices[i].position;
			arrowVertices_.push_back(position);
			lower.x = position.x < lower.x ? position.x : lower.x;
			lower.y = position.y < lower.y ? position.y : lower.y;
			lower.z = position.z < lower.z ? position.z : lower.z;
			upper.x = position.x > upper.x ? position.x : upper.x;
			upper.y = position.y > upper.y ? position.y : upper.y;
			upper.z = position.z > upper.z ? position.z : upper.z;
		}
	}
	arrowBBox_.lower = lower;
	arrowBBox_.upper = upper;
}

bool FieldRenderer::createShader()
{
	shaderProgram_ = new QOpenGLShaderProgram(this);

	const char* vertexShaderSource =
		"#version 330 core\n"
		"layout(location = 0) in vec3 aPos;\n"
		"layout(location = 1) in vec3 aColor;\n"
		"layout(location = 2) in mat4 aModelMatrix;\n"
		"out vec3 fColor;\n"
		"uniform mat4 view;\n"
		"uniform mat4 projection;\n"
		"void main() {\n"
		"   gl_Position = projection * view * aModelMatrix * vec4(aPos, 1.0f);\n"
		"   fColor = aColor;\n"
		"}\n";

	const char* fragmentShaderSource =
		"#version 330 core\n"
		"out vec4 fragColor;\n"
		"in vec3 fColor;\n"
		"void main() {\n"
		"   fragColor = vec4(fColor, 1.0f);\n"
		"}\n";

	bool success = shaderProgram_->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
	if (!success)
	{
		qDebug() << "shaderProgram addShaderFromSourceFile failed!" << shaderProgram_->log();
		return success;
	}
	success = shaderProgram_->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
	if (!success)
	{
		qDebug() << "shaderProgram addShaderFromSourceFile failed!" << shaderProgram_->log();
		return success;
	}
	success = shaderProgram_->link();
	if (!success)
	{
		qDebug() << "shaderProgram link failed!" << shaderProgram_->log();
	}
	return success;
}

QVector3D FieldRenderer::getCameraPosition()
{
	QVector3D front;  
	front[0] = cos(radians(yaw_)) * cos(radians(pitch_));
	front[1] = sin(radians(pitch_));
	front[2] = sin(radians(yaw_)) * cos(radians(pitch_));
	front.normalize();
	front = front * cameraDistance_;
	QVector3D position = lookAt_ - front;
	return position;
}

float FieldRenderer::radians(float degree) 
{
	return agz::math::deg2rad(degree);
}

void FieldRenderer::setSampleType(SampleType sampleType)
{
	sampleType_ = sampleType;
}

void FieldRenderer::samplePoints(const AABB& bbox, long sampleNum)
{
	if (sampleType_ == RANDOMSAMPLE)
		randomSamplePoints(bbox, sampleNum);
	else if (sampleType_ == UNIFORMSAMPLE)
		uniformSamplePoints(bbox, sampleNum);
	else if (sampleType_ == HALTONSAMPLE)
		haltonSamplePoints(bbox, sampleNum);
}

void FieldRenderer::randomSamplePoints(const AABB& bbox, long sampleNum)
{
	std::random_device seed;
	std::mt19937_64 gen(seed());
	std::uniform_real_distribution<> distX(bbox.lower.x, bbox.upper.x),
		distY(bbox.lower.y, bbox.upper.y), distZ(bbox.lower.z, bbox.upper.z);
	int i = 0;
	velPointsSamples_.clear();
	velocitySamples_.clear();
	while (i < sampleNum)
	{
		Vec3 point;
		point.x = distX(gen);
		point.y = distY(gen);
		point.z = distZ(gen);
		auto optVelocity = velocityField_->getVelocity(point);
		if (!optVelocity) continue;
		Vec3 velocity = *optVelocity;
		velPointsSamples_.push_back(point);
		velocitySamples_.push_back(velocity);
		i++;
	}
}

void FieldRenderer::uniformSamplePoints(const AABB& bbox, long sampleNum)
{
	std::random_device seed;
	std::mt19937_64 gen(seed());
	std::uniform_real_distribution<> dist(0, 1);
	velPointsSamples_.clear();
	velocitySamples_.clear();
	const Vec3 extent = bbox.upper - bbox.lower;
	float vBBox = extent.product();
	float theta = pow(1.0 * sampleNum / vBBox, 1.0 / 3);
	int numX = ceil(theta * (bbox.upper.x - bbox.lower.x));
	int numY = ceil(theta * (bbox.upper.y - bbox.lower.y));
	int numZ = ceil(theta * (bbox.upper.z - bbox.lower.z));
	const float boxSidelen = (extent / Vec3(numX, numY, numZ)).max_elem();
	float delta = 1.0 / theta;
	for (int i = 0; i < numX; i++)
	{
		for (int j = 0; j < numY; j++)
		{
			for (int k = 0; k < numZ; k++)
			{
				for (int s = 0; s < 10; s++)
				{
					Vec3 point;
					point.x = bbox.lower.x + (i + dist(gen)) * boxSidelen; //i * delta + delta * dist(gen);
					point.y = bbox.lower.y + (j + dist(gen)) * boxSidelen; //j * delta + delta * dist(gen);
					point.z = bbox.lower.z + (k + dist(gen)) * boxSidelen; //k * delta + delta * dist(gen);
					auto optVelocity = velocityField_->getVelocity(point);
					if (!optVelocity)
						continue;
					else
					{
						Vec3 velocity = *optVelocity;
						velPointsSamples_.push_back(point);
						velocitySamples_.push_back(velocity);
						break;
					}
				}
				
			}
		}
	}
}

void FieldRenderer::haltonSamplePoints(const AABB& bbox, long sampleNum)
{
	int i = 1;
	velPointsSamples_.clear();
	velocitySamples_.clear();
	float dx = bbox.upper.x - bbox.lower.x;
	float dy = bbox.upper.y - bbox.lower.y;
	float dz = bbox.upper.z - bbox.lower.z;
	while (true)
	{
		Vec3 point;
		point.x = bbox.lower.x + dx * radicalInverseFunction(i, 2);
		point.y = bbox.lower.y + dy * radicalInverseFunction(i, 3);
		point.z = bbox.lower.z + dz * radicalInverseFunction(i, 5);
		auto optVelocity = velocityField_->getVelocity(point);
		i++;
		if (!optVelocity) continue;
		Vec3 velocity = *optVelocity;
		velPointsSamples_.push_back(point);
		velocitySamples_.push_back(velocity);
		if (velPointsSamples_.size() >= sampleNum)
			break;
	}

}

float FieldRenderer::radicalInverseFunction(int a, int b)
{
	float f = 1;
	float r = 0;
	while (a > 0)
	{
		f = f / b;
		r = r + f * (a % b);
		a = a / b;
	}
	return r;
}

void FieldRenderer::constructInstanceData(HSVColorMapper* colorMapper, Vec3 scale)
{
	instanceDatas_.clear();
	for (int i = 0; i < velPointsSamples_.size(); i++) 
	{
		Mat4 modelMatrix = getModelMatrix(velPointsSamples_[i], velocitySamples_[i], scale);
		QColor qColor = colorMapper->getColor(velocitySamples_[i]);
		Vec3 color(qColor.redF(), qColor.greenF(), qColor.blueF());
		instanceDatas_.push_back(InstanceData(color, modelMatrix));
	}
}

Mat4 FieldRenderer::getModelMatrix(Vec3 point, Vec3 velocity, Vec3 scale)
{
	Mat4 modelMatrix;
	Mat4 scaleMatrix = Mat4::left_transform::scale(scale);
	Mat4 translateMartix = Mat4::left_transform::translate(point);
	auto frame = agz::math::tcoord3<float>::from_y(velocity);
	Vec4 rotateX = Vec4(frame.x[0], frame.x[1], frame.x[2], 0);
	Vec4 rotateY = Vec4(frame.y[0], frame.y[1], frame.y[2], 0);
	Vec4 rotateZ = Vec4(frame.z[0], frame.z[1], frame.z[2], 0);
	Vec4 rotateW = Vec4(0, 0, 0, 1);
	Mat4 rotateMatrix = Mat4::from_cols(rotateX, rotateY, rotateZ, rotateW);
	modelMatrix = translateMartix * rotateMatrix * scaleMatrix;
	return modelMatrix;
}

float FieldRenderer::getArrowSize(AABB bbox1, AABB bbox2, long sampleNum) 
{
	Vec3 v1 = bbox1.upper - bbox1.lower;
	float vBBox1 = abs(v1.x * v1.y * v1.z);
	Vec3 v2 = bbox2.upper - bbox2.lower;
	float vBBox2 = abs(v2.x * v2.y * v2.z);
	float scale = vBBox1 / (vBBox2 * sampleNum);
	return scale;
}

void FieldRenderer::setArrowSize(float x, float y, float z)
{
	scaleResize_ =  Vec3(x, y, z);
}

void FieldRenderer::setArrowXSize(float x)
{
	scaleResize_ =  Vec3(x, 1.0, 1.0);
}

void FieldRenderer::setArrowYSize(float y)
{
	scaleResize_ =  Vec3(1.0, y, 1.0);
}

void FieldRenderer::setArrowZSize(float z)
{
	scaleResize_ = Vec3(1.0, 1.0, z);
}

void FieldRenderer::renderForReScaleArrow()
{
	updateInstanceOfMatrix(scale_ * scaleResize_);
	updataInstanceVBO();
	update();
}

float FieldRenderer::getArrowScale(int axis)
{
	if (axis == 0)
		return scaleResize_.x;
	else if (axis == 1)
		return scaleResize_.y;
	else if (axis == 2)
		return scaleResize_.z;
	else
		return scaleResize_.length();
}

void FieldRenderer::updateInstanceOfMatrix(Vec3 scale)
{
	for (int i = 0; i < instanceDatas_.size(); i++)
	{
		Mat4 modelMatrix = getModelMatrix(velPointsSamples_[i], velocitySamples_[i], scale);
		instanceDatas_[i].modelMatrix_ = modelMatrix;
	}
}

long FieldRenderer::getSampleNums()
{
	return sampleNum_;
}

void FieldRenderer::setSampleNums(long sampleNum)
{
	sampleNum_ = sampleNum;
}

void FieldRenderer::renderForReSample()
{
	samplePoints(velocityFieldBBox_, sampleNum_);
	constructInstanceData(colorMapper_, scale_ * scaleResize_);
	updataInstanceVBO();
	update();
}

void FieldRenderer::updateInstanceofColor()
{
	for (int i = 0; i < instanceDatas_.size(); i++)
	{
		QColor qColor = colorMapper_->getColor(velocitySamples_[i]);
		Vec3 color(qColor.redF(), qColor.greenF(), qColor.blueF());
		instanceDatas_[i].color_ = color;
	}
	updataInstanceVBO();
	update();
}

void FieldRenderer::updataInstanceVBO()
{
	makeCurrent();
	glDeleteBuffers(1, &instanceVBO_);
	glBindVertexArray(vao_);
	bindInstanceVBOForPaint();
	doneCurrent();
}

void FieldRenderer::bindInstanceVBOForPaint()
{
	glGenBuffers(1, &instanceVBO_);
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO_);
	glBufferData(GL_ARRAY_BUFFER, instanceDatas_.size() * sizeof(InstanceData), instanceDatas_.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)offsetof(InstanceData, color_));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)offsetof(InstanceData, modelMatrix_));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)(offsetof(InstanceData, modelMatrix_) + 4 * sizeof(float)));
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)(offsetof(InstanceData, modelMatrix_) + 8 * sizeof(float)));
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)(offsetof(InstanceData, modelMatrix_) + 12 * sizeof(float)));
	glVertexAttribDivisor(1, 1);
	glVertexAttribDivisor(2, 1);
	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
