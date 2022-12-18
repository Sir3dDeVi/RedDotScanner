#include "pch.h"
#include "SetupView.xaml.h"
#if __has_include("SetupView.g.cpp")
#include "SetupView.g.cpp"
#endif

#include "IntrinsicCalibrationDialog.xaml.h"
#include "ExtrinsicCalibrationDialog.xaml.h"


using namespace winrt;
using namespace Microsoft::UI::Xaml;


namespace winrt::RedDotScanner::implementation
{
    SetupView::SetupView()
    {
        InitializeComponent();
    }

    winrt::RedDotScanner::MainViewModel SetupView::ViewModel()
    {
        return m_rtMainViewModel;
    }
    void SetupView::ViewModel(winrt::RedDotScanner::MainViewModel const& value)
    {
        m_rtMainViewModel = value;
    }

    Windows::Foundation::IAsyncAction SetupView::IntrinsicCalibration_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        auto rtElement = sender.as<Microsoft::UI::Xaml::FrameworkElement>();
        RedDotScanner::Camera rtCamera = rtElement.Tag().as<RedDotScanner::Camera>(); // I'm not happy about using Tag, but this was the quickest solution

        rtCamera.PreviewType(RedDotScanner::PreviewType::Color);

        auto rtDialog = winrt::make<IntrinsicCalibrationDialog>(this->XamlRoot(), rtCamera);
        auto rtResult = co_await rtDialog.ShowAsync();

        if (rtResult == Microsoft::UI::Xaml::Controls::ContentDialogResult::Primary) {
            rtCamera.PreviewType(RedDotScanner::PreviewType::Undistorted);
        }
    }

    void SetupView::ExtrinsicCalibration_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        auto rtElement = sender.as<Microsoft::UI::Xaml::FrameworkElement>();
        RedDotScanner::Camera rtCamera = rtElement.Tag().as<RedDotScanner::Camera>(); // I'm not happy about using Tag, but this was the quickest solution

        rtCamera.PreviewType(RedDotScanner::PreviewType::Color);

        auto rtDialog = winrt::make<ExtrinsicCalibrationDialog>(this->XamlRoot(), rtCamera);
        rtDialog.ShowAsync();
    }

    void SetupView::PreviewColor_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        auto rtElement = sender.as<Microsoft::UI::Xaml::FrameworkElement>();
        RedDotScanner::Camera rtCamera = rtElement.Tag().as<RedDotScanner::Camera>();

        rtCamera.PreviewType(RedDotScanner::PreviewType::Color);
    }

    void SetupView::PreviewUndistorted_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        auto rtElement = sender.as<Microsoft::UI::Xaml::FrameworkElement>();
        RedDotScanner::Camera rtCamera = rtElement.Tag().as<RedDotScanner::Camera>();

        rtCamera.PreviewType(RedDotScanner::PreviewType::Undistorted);
    }

    void SetupView::PreviewThresholded_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        auto rtElement = sender.as<Microsoft::UI::Xaml::FrameworkElement>();
        RedDotScanner::Camera rtCamera = rtElement.Tag().as<RedDotScanner::Camera>();

        rtCamera.PreviewType(RedDotScanner::PreviewType::Thresholded);
    }
}
