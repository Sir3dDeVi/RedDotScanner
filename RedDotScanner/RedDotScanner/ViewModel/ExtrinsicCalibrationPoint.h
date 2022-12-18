#pragma once
#include "ExtrinsicCalibrationPoint.g.h"

#include "NotifyPropretyChangedImpl.h"


namespace winrt::RedDotScanner::implementation
{
    struct ExtrinsicCalibrationPoint
        : ExtrinsicCalibrationPointT<ExtrinsicCalibrationPoint>
        , NotifyPropertyChangedImplT<ExtrinsicCalibrationPoint>
    {
        ExtrinsicCalibrationPoint();

        float ImageX();
        void ImageX(float value);

        float ImageY();
        void ImageY(float value);

        float WorldX();
        void WorldX(float value);

        float WorldY();
        void WorldY(float value);

        float WorldZ();
        void WorldZ(float value);

    private:
        float m_ImageX, m_ImageY;
        float m_WorldX, m_WorldY, m_WorldZ;
    };
}
namespace winrt::RedDotScanner::factory_implementation
{
    struct ExtrinsicCalibrationPoint : ExtrinsicCalibrationPointT<ExtrinsicCalibrationPoint, implementation::ExtrinsicCalibrationPoint>
    {
    };
}
