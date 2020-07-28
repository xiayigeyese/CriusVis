#pragma once

#include <vtkDataSet.h>
#include <vtkDoubleArray.h>
#include <vtkFLUENTReader.h>
#include <vtkSmartPointer.h>
#include <vtkStaticCellLocator.h>

#include <crius/velocityField/velocityField.h>

/**
 * @brief velocity field loaded from cas file exported by Fluent
 */
class FluentVelocityField : public VelocityField
{
public:

    explicit FluentVelocityField(const std::string &filename);

    std::optional<Vec3> getVelocity(const Vec3 &pos) const noexcept override;

    float getMaxVelocity(VelocityComponent component) const noexcept override;

    float getMinVelocity(VelocityComponent component) const noexcept override;

    AABB getBoundingBox() const noexcept override;

    RC<VelocityField> cloneForParallelAccess() const override;

private:

    FluentVelocityField() { }

    vtkSmartPointer<vtkFLUENTReader> reader_;

    Vec3 maxDirVel_;
    Vec3 minDirVel_;

    float maxVel_;
    float minVel_;

    vtkDoubleArray *velX_;
    vtkDoubleArray *velY_;
    vtkDoubleArray *velZ_;

    vtkSmartPointer<vtkStaticCellLocator> cellLocator_;

    vtkDataSet *dataSet_;
};
