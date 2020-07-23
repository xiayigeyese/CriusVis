#include <vtkMultiBlockDataSet.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>

#include <crius/core/fluentVelocityField.h>

FluentVelocityField::FluentVelocityField(const std::string &filename)
{
    reader_ = vtkSmartPointer<vtkFLUENTReader>::New();
    reader_->SetFileName(filename.c_str());
    reader_->EnableAllCellArrays();
    reader_->Update();

    auto dataSet = dynamic_cast<vtkDataSet *>(
        reader_->GetOutput()->GetBlock(0));

    const int numCells = static_cast<int>(dataSet->GetNumberOfCells());

    std::vector<vtkCell *> cells;
    cells.reserve(numCells);

    for(int i = 0; i < numCells; ++i)
        cells.push_back(dataSet->GetCell(i));

    // TODO: build bvh with cells

    auto cellData = dataSet->GetCellData();
    velX_ = dynamic_cast<vtkDoubleArray*>(cellData->GetArray("VELOCITY_X"));
    velY_ = dynamic_cast<vtkDoubleArray*>(cellData->GetArray("VELOCITY_Y"));
    velZ_ = dynamic_cast<vtkDoubleArray*>(cellData->GetArray("VELOCITY_Z"));
}

std::optional<Vec3> FluentVelocityField::getVelocity(const Vec3 &pos) const noexcept
{
    // TODO
    return std::nullopt;
}

float FluentVelocityField::getMaxVelocity(VelocityComponent component) const noexcept
{
    // TODO
    return 0;
}

float FluentVelocityField::getMinVelocity(VelocityComponent component) const noexcept
{
    // TODO
    return 0;
}

AABB FluentVelocityField::getBoundingBox() const noexcept
{
    // TODO
    return {};
}
