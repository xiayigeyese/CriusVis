#pragma once

#include <vtkDoubleArray.h>
#include <vtkFLUENTReader.h>
#include <vtkSmartPointer.h>

#include <crius/core/velocityField.h>

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

private:

    vtkSmartPointer<vtkFLUENTReader> reader_;

    vtkDoubleArray *velX_;
    vtkDoubleArray *velY_;
    vtkDoubleArray *velZ_;
};
