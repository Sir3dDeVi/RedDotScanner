#include "pch.h"
#include "DoubleToStringConverter.h"
#include "DoubleToStringConverter.g.cpp"

#include <iomanip>
#include <sstream>


namespace winrt::RedDotScanner::implementation
{
    winrt::Windows::Foundation::IInspectable DoubleToStringConverter::Convert(winrt::Windows::Foundation::IInspectable const& value, winrt::Windows::UI::Xaml::Interop::TypeName const& /*targetType*/, winrt::Windows::Foundation::IInspectable const& /*parameter*/, hstring const& /*language*/)
    {
        double number = winrt::unbox_value_or<double>(value, -1.0);

        std::wostringstream ss;
        ss << std::fixed << std::setprecision(1) << number;

        return winrt::box_value(ss.str());
    }
    winrt::Windows::Foundation::IInspectable DoubleToStringConverter::ConvertBack(winrt::Windows::Foundation::IInspectable const& /*value*/, winrt::Windows::UI::Xaml::Interop::TypeName const& /*targetType*/, winrt::Windows::Foundation::IInspectable const& /*parameter*/, hstring const& /*language*/)
    {
        throw hresult_not_implemented();
    }
}
