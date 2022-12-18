#include "pch.h"
#include "ExtrinsicCalibrationPoint.h"
#include "ExtrinsicCalibrationPoint.g.cpp"


namespace winrt::RedDotScanner::implementation
{
    ExtrinsicCalibrationPoint::ExtrinsicCalibrationPoint()
        : m_ImageX(0.0f)
        , m_ImageY(0.0f)
        , m_WorldX(0.0f)
        , m_WorldY(0.0f)
        , m_WorldZ(0.0f)
    {
    }

    float ExtrinsicCalibrationPoint::ImageX()
    {
        return m_ImageX;
    }

    void ExtrinsicCalibrationPoint::ImageX(float value)
    {
        m_ImageX = value;
        RaisePropertyChanged(L"ImageX");
    }

    float ExtrinsicCalibrationPoint::ImageY()
    {
        return m_ImageY;
    }

    void ExtrinsicCalibrationPoint::ImageY(float value)
    {
        m_ImageY = value;
        RaisePropertyChanged(L"ImageY");
    }

    float ExtrinsicCalibrationPoint::WorldX()
    {
        return m_WorldX;
    }

    void ExtrinsicCalibrationPoint::WorldX(float value)
    {
        m_WorldX = value;
        RaisePropertyChanged(L"WorldX");
    }

    float ExtrinsicCalibrationPoint::WorldY()
    {
        return m_WorldY;
    }

    void ExtrinsicCalibrationPoint::WorldY(float value)
    {
        m_WorldY = value;
        RaisePropertyChanged(L"WorldY");
    }

    float ExtrinsicCalibrationPoint::WorldZ()
    {
        return m_WorldZ;
    }

    void ExtrinsicCalibrationPoint::WorldZ(float value)
    {
        m_WorldZ = value;
        RaisePropertyChanged(L"WorldZ");
    }
}
