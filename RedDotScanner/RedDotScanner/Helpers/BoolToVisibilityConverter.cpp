#include "pch.h"
#include "BoolToVisibilityConverter.h"
#include "BoolToVisibilityConverter.g.cpp"


namespace winrt::RedDotScanner::implementation
{
    winrt::Windows::Foundation::IInspectable BoolToVisibilityConverter::Convert(winrt::Windows::Foundation::IInspectable const& value, winrt::Windows::UI::Xaml::Interop::TypeName const& /*targetType*/, winrt::Windows::Foundation::IInspectable const& /*parameter*/, hstring const& /*language*/)
    {
        bool visible = winrt::unbox_value_or<bool>(value, false);

        return winrt::box_value(visible ? Microsoft::UI::Xaml::Visibility::Visible : Microsoft::UI::Xaml::Visibility::Collapsed);
    }
    winrt::Windows::Foundation::IInspectable BoolToVisibilityConverter::ConvertBack(winrt::Windows::Foundation::IInspectable const& /*value*/, winrt::Windows::UI::Xaml::Interop::TypeName const& /*targetType*/, winrt::Windows::Foundation::IInspectable const& /*parameter*/, hstring const& /*language*/)
    {
        throw hresult_not_implemented();
    }
}
