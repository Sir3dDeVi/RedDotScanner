#pragma once

#include "winrt/Microsoft.UI.Xaml.h"
#include "winrt/Microsoft.UI.Xaml.Markup.h"
#include "winrt/Microsoft.UI.Xaml.Controls.Primitives.h"
#include "CaptureView.g.h"

#include "Direct3DPanel.h"


namespace winrt::RedDotScanner::implementation
{
    struct CaptureView : CaptureViewT<CaptureView>
    {
        CaptureView();

        winrt::RedDotScanner::MainViewModel ViewModel();
        void ViewModel(winrt::RedDotScanner::MainViewModel const& value);

        RedDotScanner::WorldView WorldView();

        void ScanArea_Set(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ScanArea_GoTo(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& e);

        void StartScan_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void StopScan_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& e);

        void BundleAdjustment_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void Leveling_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ExportToObj_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& e);

    private:
        RedDotScanner::MainViewModel m_rtMainViewModel { nullptr };
    };
}

namespace winrt::RedDotScanner::factory_implementation
{
    struct CaptureView : CaptureViewT<CaptureView, implementation::CaptureView>
    {
    };
}
