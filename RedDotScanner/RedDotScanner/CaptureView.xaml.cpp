#include "pch.h"
#include "CaptureView.xaml.h"
#if __has_include("CaptureView.g.cpp")
#include "CaptureView.g.cpp"
#endif

#include <iomanip>
#include <Utils.h>

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::RedDotScanner::implementation
{
    CaptureView::CaptureView()
    {
        InitializeComponent();
    }

    winrt::RedDotScanner::MainViewModel CaptureView::ViewModel()
    {
        return m_rtMainViewModel;
    }

    void CaptureView::ViewModel(winrt::RedDotScanner::MainViewModel const& value)
    {
        m_rtMainViewModel = value;
    }

    RedDotScanner::WorldView CaptureView::WorldView()
    {
        return WorldViewControl();
    }


    void CaptureView::ScanArea_Set(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        auto rtElement = sender.as<Microsoft::UI::Xaml::FrameworkElement>();
        RedDotScanner::PanTilt rtPanTilt = rtElement.Tag().as<RedDotScanner::PanTilt>();

        RedDotScanner::LaserTurret rtLaserTurret = m_rtMainViewModel.Scanner().LaserTurret();

        rtPanTilt.Pan(rtLaserTurret.Pan());
        rtPanTilt.Tilt(rtLaserTurret.Tilt());
    }

    void CaptureView::ScanArea_GoTo(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        auto rtElement = sender.as<Microsoft::UI::Xaml::FrameworkElement>();
        RedDotScanner::PanTilt rtPanTilt = rtElement.Tag().as<RedDotScanner::PanTilt>();

        RedDotScanner::LaserTurret rtLaserTurret = m_rtMainViewModel.Scanner().LaserTurret();

        rtLaserTurret.RotateTo(
            rtPanTilt.Pan(), 30,
            rtPanTilt.Tilt(), 30
        );
    }

    void CaptureView::StartScan_Click(Windows::Foundation::IInspectable const& /*sender*/, Microsoft::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        m_rtMainViewModel.Scanner().StartScanning();
    }

    void CaptureView::StopScan_Click(Windows::Foundation::IInspectable const& /*sender*/, Microsoft::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        m_rtMainViewModel.Scanner().StopScanning();
    }

    void CaptureView::BundleAdjustment_Click(Windows::Foundation::IInspectable const& /*sender*/, Microsoft::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        auto rtResult = m_rtMainViewModel.Scanner().RunBundleAdjustment();

        Microsoft::UI::Xaml::Controls::ContentDialog dlg;
        dlg.XamlRoot(XamlRoot());
        dlg.Title(winrt::box_value(L"Bundle Adjustment"));
        dlg.Content(winrt::box_value(winrt::to_hstring(static_cast<std::string>(utils::S()
            << "Initial reprojection error: " << std::fixed << std::setprecision(3) << rtResult.InitialReprojectionError() << "\n"
            << "Calibrated reprojection error: " << std::fixed << std::setprecision(3) << rtResult.CalibrationReprojectionError()
        ))));
        dlg.CloseButtonText(L"Ok");

        dlg.ShowAsync();
    }

    void CaptureView::Leveling_Click(Windows::Foundation::IInspectable const& /*sender*/, Microsoft::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        m_rtMainViewModel.Scanner().DoLeveling();
    }

    void CaptureView::ExportToObj_Click(Windows::Foundation::IInspectable const& /*sender*/, Microsoft::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        m_rtMainViewModel.Scanner().SaveScanToObj(L"scan.obj");
    }

}
