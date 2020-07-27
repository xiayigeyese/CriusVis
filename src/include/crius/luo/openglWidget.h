#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLFunctions_3_3_Core>
#include <QVector3D>
#include <QMatrix4x4>

#include <crius/core/fluentVelocityField.h>
#include <crius/core/constantVelocityField.h>
#include <crius/ui/hsvColorMapper.h>
#include <crius/common.h>

class QOpenGLShaderProgram;

class OpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
	Q_OBJECT;
public:
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
	OpenGLWidget(QWidget* parent = 0);
	~OpenGLWidget();

	void setVelcotityField(VelocityField* velocityField);

protected:
	void initializeGL() override;
	void paintGL() override;
	void resizeGL(int width, int height) override;
 
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
	void samplePoints(AABB bbox, long maxSampleNum);
	void constructInstanceData(HueColorMapper *colorMapper, Vec3 scale);
	Mat4 getModelMatrix(Vec3 point, Vec3 velocity, Vec3 scale);
	
	float getArrowSize(AABB bbox1, AABB bbox2, long sampleNum);
	void setArrowXSize(float x);
	void setArrowYSize(float y);
	void setArrowZSize(float z);
	void setArrowSize(float x, float y, float z);
	void setSampleNums(long sampleNum);

	void updateInstanceOfMatrix(Vec3 scale);
	void updateInstanceofColor();
	void updataInstanceVBO();
	void bindInstanceVBOForPaint();

private:
	// shader 成员
	QOpenGLShaderProgram* shaderProgram_;
	unsigned int vao_, vbo_, instanceVBO_;

	// 箭头模型点集
	QVector<Vec3> arrowVertices_;
	
	// 摄像机设置
	QVector3D velocityFiledCenter_, lookAt_, eye_;
	bool leftBtnPressed_, rightBtnPressed_, middleBtnPressed_;
	QPoint lastPos_;
	float yaw_, pitch_, cameraDistance_, fov_;
	
    // instanceData 成员
	QVector<InstanceData> instanceDatas_;
	QVector<Vec3> velPointsSamples_, velocitySamples_;
	/*FluentVelocityField* velocityField_;
	ConstantVelocityField * velocityField3_;*/
	VelocityField* velocityField_;
	HueColorMapper *colorMapper_;
	AABB velocityFieldBBox_, arrowBBox_;

	// 影响instanceData的可变参数
	long sampleNum_;
	Vec3 scale_;

	// test
	bool firstFlag_;
};