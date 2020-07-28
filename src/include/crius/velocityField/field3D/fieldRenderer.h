#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLFunctions_3_3_Core>
#include <QVector3D>
#include <QMatrix4x4>

#include <crius/velocityField/fluentVelocityField.h>
#include <crius/velocityField/constantVelocityField.h>
#include <crius/common/hsvColorMapper.h>

class QOpenGLShaderProgram;

class FieldRenderer : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
	Q_OBJECT

public:

	enum SampleType
	{
		RANDOMSAMPLE = 0,
		UNIFORMSAMPLE = 1,
	    HALTONSAMPLE = 2
	};

	struct InstanceData
	{
		Vec3 color_;
		Mat4 modelMatrix_;

		InstanceData() {}
		InstanceData(Vec3 color, Mat4 modelMatrix) 
			: color_(color), modelMatrix_(modelMatrix)
		{

		}
	};

	FieldRenderer(QWidget* parent, const VelocityField* velocityField, HSVColorMapper* colorMapper);
	~FieldRenderer();

	void setVelcotityField(VelocityField* velocityField);
	
	void setArrowXSize(float x);
	void setArrowYSize(float y);
	void setArrowZSize(float z);
	void setArrowSize(float x, float y, float z);
	float getArrowScale(int axis);

	void setSampleType(SampleType sampleType);
	void setSampleNums(long sampleNum);
	long getSampleNums();

	void renderForReSample();
	void renderForReScaleArrow();

protected:

	void initializeGL() override;
	void paintGL() override;
 
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void wheelEvent(QWheelEvent* event) override;

private:

	void init();
	bool createShader();
	void loadArrowMesh();
	float radians(float degree);
	QVector3D getCameraPosition();

	void samplePoints(const AABB& bbox, long sampleNum);
	void randomSamplePoints(const AABB& bbox, long sampleNum);
	void uniformSamplePoints(const AABB& bbox, long sampleNum);
	void haltonSamplePoints(const AABB& bbox, long sampleNum);
	static float radicalInverseFunction(int a, int b);

	void constructInstanceData(HSVColorMapper *colorMapper, Vec3 scale);
	Mat4 getModelMatrix(Vec3 point, Vec3 velocity, Vec3 scale);

	float getArrowSize(AABB bbox1, AABB bbox2, long sampleNum);

	void updateInstanceOfMatrix(Vec3 scale);
	void updateInstanceofColor();
	void updataInstanceVBO();
	void bindInstanceVBOForPaint();

	// shader members
	QOpenGLShaderProgram* shaderProgram_;
	unsigned int vao_, vbo_, instanceVBO_;

	// arrow model
	QVector<Vec3> arrowVertices_;
	
	// camera settings
	QVector3D velocityFiledCenter_, lookAt_, eye_;
	bool leftBtnPressed_, rightBtnPressed_, middleBtnPressed_;
	QPoint lastPos_;
	float yaw_, pitch_, cameraDistance_, fov_;
	
    // instancing
	QVector<InstanceData> instanceDatas_;
	QVector<Vec3> velPointsSamples_, velocitySamples_;
	const VelocityField* velocityField_;
	HSVColorMapper *colorMapper_;
	AABB velocityFieldBBox_, arrowBBox_;

	// instancing settings
	long sampleNum_;
	SampleType sampleType_;
	Vec3 scale_, scaleResize_;
};
