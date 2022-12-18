#include "pch.h"
#include "NXTDeviceInfo.h"
#include "NXTDeviceInfo.g.cpp"


namespace winrt::RedDotScanner::implementation
{
    NXTDeviceInfo::NXTDeviceInfo(hstring const& rtName, hstring const& rtResourceName)
        : m_rtName{ rtName }
        , m_rtResourceName{ rtResourceName }
    {
    }

    hstring NXTDeviceInfo::Name()
    {
        return m_rtName;
    }

    hstring NXTDeviceInfo::ResourceName()
    {
        return m_rtResourceName;
    }
}
