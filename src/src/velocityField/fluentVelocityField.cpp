#include <iostream>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkMultiBlockDataSet.h>

#include <crius/velocityField/fluentVelocityField.h>

FluentVelocityField::FluentVelocityField(const std::string &filename)
{
    reader_ = vtkSmartPointer<vtkFLUENTReader>::New();
    reader_->SetFileName(filename.c_str());
    reader_->EnableAllCellArrays();
    reader_->Update();

    dataSet_ = dynamic_cast<vtkDataSet *>(
        reader_->GetOutput()->GetBlock(0));

    cellLocator_ = vtkSmartPointer<vtkStaticCellLocator>::New();
    cellLocator_->SetDataSet(dataSet_);
    cellLocator_->BuildLocator();

    const int numCells = static_cast<int>(dataSet_->GetNumberOfCells());

    std::cout << "Cells in " << filename << ": " << numCells << std::endl;

    auto cellData = dataSet_->GetCellData();

    velX_ = dynamic_cast<vtkDoubleArray*>(cellData->GetArray("X_VELOCITY"));
    velY_ = dynamic_cast<vtkDoubleArray*>(cellData->GetArray("Y_VELOCITY"));
    velZ_ = dynamic_cast<vtkDoubleArray*>(cellData->GetArray("Z_VELOCITY"));

    if(!velX_ || !velY_ || !velZ_)
    {
        throw std::runtime_error(
            "invalid to load fluent velocity field from " + filename);
    }

    maxDirVel_ = Vec3(std::numeric_limits<float>::lowest());
    minDirVel_ = Vec3(std::numeric_limits<float>::max());
    maxVel_ = std::numeric_limits<float>::lowest();
    minVel_ = std::numeric_limits<float>::max();

    for(int i = 0; i < numCells; ++i)
    {
        const float x = static_cast<float>(velX_->GetValue(i));
        const float y = static_cast<float>(velY_->GetValue(i));
        const float z = static_cast<float>(velZ_->GetValue(i));

        maxDirVel_.x = (std::max)(maxDirVel_.x, x);
        maxDirVel_.y = (std::max)(maxDirVel_.y, y);
        maxDirVel_.z = (std::max)(maxDirVel_.z, z);

        minDirVel_.x = (std::min)(minDirVel_.x, x);
        minDirVel_.y = (std::min)(minDirVel_.y, y);
        minDirVel_.z = (std::min)(minDirVel_.z, z);

        const float len = std::sqrt(x * x + y * y + z * z);
        maxVel_ = (std::max)(maxVel_, len);
        minVel_ = (std::min)(minVel_, len);
    }
}

std::optional<Vec3> FluentVelocityField::getVelocity(
    const Vec3 &pos) const noexcept
{
    double x[3] = { pos.x, pos.y, pos.z };
    const auto cellID = cellLocator_->FindCell(x);

    if(cellID < 0)
        return std::nullopt;

    return Vec3(
        velX_->GetValue(cellID),
        velY_->GetValue(cellID),
        velZ_->GetValue(cellID));
}

float FluentVelocityField::getMaxVelocity(
    VelocityComponent component) const noexcept
{
    if(component == All)
        return maxVel_;
    return maxDirVel_[component];
}

float FluentVelocityField::getMinVelocity(
    VelocityComponent component) const noexcept
{
    if(component == All)
        return minVel_;
    return minDirVel_[component];
}

AABB FluentVelocityField::getBoundingBox() const noexcept
{
    double bounds[6];
    dataSet_->GetBounds(bounds);
    return {
        {
            static_cast<float>(bounds[0]),
            static_cast<float>(bounds[2]),
            static_cast<float>(bounds[4])
        },
        {
            static_cast<float>(bounds[1]),
            static_cast<float>(bounds[3]),
            static_cast<float>(bounds[5])
        }
    };
}

RC<VelocityField> FluentVelocityField::cloneForParallelAccess() const
{
    auto ret = RC<FluentVelocityField>(new FluentVelocityField);

    ret->reader_    = reader_;
    ret->maxDirVel_ = maxDirVel_;
    ret->minDirVel_ = minDirVel_;
    ret->maxVel_    = maxVel_;
    ret->minVel_    = minVel_;
    ret->velX_      = velX_;
    ret->velY_      = velY_;
    ret->velZ_      = velZ_;
    ret->dataSet_   = dataSet_;

    ret->cellLocator_ = vtkSmartPointer<vtkStaticCellLocator>::New();
    ret->cellLocator_->SetDataSet(dataSet_);
    ret->cellLocator_->BuildLocator();

    return ret;
}
