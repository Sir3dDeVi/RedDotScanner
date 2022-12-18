#include "pch.h"
#include "PanTilt.h"
#include "PanTilt.g.cpp"


namespace winrt::RedDotScanner::implementation
{
    PanTilt::PanTilt()
        : m_Pan(0)
        , m_Tilt(0)
    {
    }

    PanTilt::PanTilt(int32_t pan, int32_t tilt)
        : m_Pan(pan)
        , m_Tilt(tilt)
    {
    }

    int32_t PanTilt::Pan()
    {
        return m_Pan;
    }

    void PanTilt::Pan(int32_t value)
    {
        m_Pan = value;

        RaisePropertyChanged(L"Pan");
    }

    int32_t PanTilt::Tilt()
    {
        return m_Tilt;
    }

    void PanTilt::Tilt(int32_t value)
    {
        m_Tilt = value;

        RaisePropertyChanged(L"Tilt");
    }
}
