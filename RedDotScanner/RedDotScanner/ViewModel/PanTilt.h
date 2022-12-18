#pragma once
#include "PanTilt.g.h"
#include "NotifyPropretyChangedImpl.h"


namespace winrt::RedDotScanner::implementation
{
    struct PanTilt
        : PanTiltT<PanTilt>
        , NotifyPropertyChangedImplT<PanTilt>
    {
        PanTilt();
        PanTilt(int32_t pan, int32_t tilt);

        int32_t Pan();
        void Pan(int32_t value);

        int32_t Tilt();
        void Tilt(int32_t value);

    private:
        int32_t m_Pan;
        int32_t m_Tilt;
    };
}
namespace winrt::RedDotScanner::factory_implementation
{
    struct PanTilt : PanTiltT<PanTilt, implementation::PanTilt>
    {
    };
}
