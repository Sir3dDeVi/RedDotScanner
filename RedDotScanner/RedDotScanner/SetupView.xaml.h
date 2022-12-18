#pragma once

#include "winrt/Microsoft.UI.Xaml.h"
#include "winrt/Microsoft.UI.Xaml.Markup.h"
#include "winrt/Microsoft.UI.Xaml.Controls.Primitives.h"
#include "SetupView.g.h"

#include "Helpers/DoubleToStringConverter.h"
#include "ViewModel/MainViewModel.h"


namespace winrt::RedDotScanner::implementation
{
    struct SetupView : SetupViewT<SetupView>
    {
        SetupView();

        winrt::RedDotScanner::MainViewModel ViewModel();
        void ViewModel(winrt::RedDotScanner::MainViewModel const& value);

        Windows::Foundation::IAsyncAction IntrinsicCalibration_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ExtrinsicCalibration_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void PreviewColor_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void PreviewUndistorted_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void PreviewThresholded_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& e);

    private:
        RedDotScanner::MainViewModel m_rtMainViewModel { nullptr };
    };
}

namespace winrt::RedDotScanner::factory_implementation
{
    struct SetupView : SetupViewT<SetupView, implementation::SetupView>
    {
    };
}
