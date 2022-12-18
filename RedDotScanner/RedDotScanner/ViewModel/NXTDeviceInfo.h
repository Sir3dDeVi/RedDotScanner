#pragma once
#include "NXTDeviceInfo.g.h"


namespace winrt::RedDotScanner::implementation
{
    struct NXTDeviceInfo : NXTDeviceInfoT<NXTDeviceInfo>
    {
        NXTDeviceInfo() = delete;
        NXTDeviceInfo(hstring const& rtName, hstring const& rtResourceName);

        hstring Name();
        hstring ResourceName();

    private:
        winrt::hstring m_rtName;
        winrt::hstring m_rtResourceName;
    };
}
namespace winrt::RedDotScanner::factory_implementation
{
    struct NXTDeviceInfo : NXTDeviceInfoT<NXTDeviceInfo, implementation::NXTDeviceInfo>
    {
    };
}
