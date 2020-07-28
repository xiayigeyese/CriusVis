#pragma once

#include <QWidget>
#include <QComboBox>

#include <crius/utility/doubleSlider.h>
#include <crius/velocityField/field3D/fieldRenderer.h>
#include <crius/velocityField/velocityField.h>

class VelocityField3D : public QWidget
{
	Q_OBJECT

public:

	VelocityField3D(QWidget* parent, RC<const VelocityField> velocityField);

private:

	FieldRenderer* openglWidget_;
	QComboBox* arrowSize_;
	int arrowAxis_;
	DoubleSlider* arrowSizeSlider_;

	HSVColorMapper* colorMapper_;
	ColorBar* colorBar_;

	//DoubleSlider* xminSlider_, *xmaxSlider_;
	//DoubleSlider* yminSlider_, *ymaxSlider_;
	//DoubleSlider* zminSlider_, *zmaxSlider_;

	RC<const VelocityField> velocityField_;

	QComboBox* velocitySampleType_;
};