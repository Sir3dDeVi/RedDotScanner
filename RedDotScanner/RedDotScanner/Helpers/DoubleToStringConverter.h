#pragma once
#include "DoubleToStringConverter.g.h"


namespace winrt::RedDotScanner::implementation
{
    struct DoubleToStringConverter : DoubleToStringConverterT<DoubleToStringConverter>
    {
        DoubleToStringConverter() = default;

        winrt::Windows::Foundation::IInspectable Convert(winrt::Windows::Foundation::IInspectable const& value, winrt::Windows::UI::Xaml::Interop::TypeName const& targetType, winrt::Windows::Foundation::IInspectable const& parameter, hstring const& language);
        winrt::Windows::Foundation::IInspectable ConvertBack(winrt::Windows::Foundation::IInspectable const& value, winrt::Windows::UI::Xaml::Interop::TypeName const& targetType, winrt::Windows::Foundation::IInspectable const& parameter, hstring const& language);
    };
}
namespace winrt::RedDotScanner::factory_implementation
{
    struct DoubleToStringConverter : DoubleToStringConverterT<DoubleToStringConverter, implementation::DoubleToStringConverter>
    {
    };
}
