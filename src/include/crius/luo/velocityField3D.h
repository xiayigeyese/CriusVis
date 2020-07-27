#pragma once

#include <QWidget>
#include <QComboBox>

#include <crius/ui/doubleSlider.h>
#include <crius/luo/openglWidget.h>

class VelocityField3D : public QWidget
{
	Q_OBJECT

public:
	VelocityField3D(QWidget* parent);

private:
	OpenGLWidget* openglWidget_;
	QComboBox* arrowSize_;

	HueColorMapper* colorMapper_;
	HUEColorBar* colorBar_;

	DoubleSlider* depthSlider_;
};