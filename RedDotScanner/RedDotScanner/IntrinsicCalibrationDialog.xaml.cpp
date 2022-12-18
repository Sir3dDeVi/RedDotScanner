#include "pch.h"
#include "IntrinsicCalibrationDialog.xaml.h"
#if __has_include("IntrinsicCalibrationDialog.g.cpp")
#include "IntrinsicCalibrationDialog.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;


namespace winrt::RedDotScanner::implementation
{
    IntrinsicCalibrationDialog::IntrinsicCalibrationDialog(winrt::Microsoft::UI::Xaml::XamlRoot const& rtXamlRoot, winrt::RedDotScanner::Camera const& rtCamera)
        : m_rtCamera(rtCamera)
    {
        InitializeComponent();

        this->XamlRoot(rtXamlRoot);
    }

    winrt::RedDotScanner::Camera IntrinsicCalibrationDialog::Camera()
    {
        return m_rtCamera;
    }

    Microsoft::UI::Xaml::Media::Imaging::WriteableBitmap IntrinsicCalibrationDialog::PreviewFrame()
    {
        if (m_rtSelectedFrame)
            return m_rtSelectedFrame;
        else
            return m_rtCamera.PreviewFrame();
    }

    bool IntrinsicCalibrationDialog::IsCameraMode()
    {
        return m_rtSelectedFrame == nullptr;
    }

    bool IntrinsicCalibrationDialog::IsPictureMode()
    {
        return m_rtSelectedFrame != nullptr;
    }

    Microsoft::UI::Xaml::Media::Imaging::WriteableBitmap IntrinsicCalibrationDialog::SelectedFrame()
    {
        return m_rtSelectedFrame;
    }

    void IntrinsicCalibrationDialog::SelectedFrame(Microsoft::UI::Xaml::Media::Imaging::WriteableBitmap const& value)
    {
        m_rtSelectedFrame = value;

        RaisePropertyChanged(L"IsCameraMode");
        RaisePropertyChanged(L"IsPictureMode");
        RaisePropertyChanged(L"PreviewFrame");
        RaisePropertyChanged(L"SelectedFrame");
    }

    void IntrinsicCalibrationDialog::CaptureFrame_Click(Windows::Foundation::IInspectable const& /*sender*/, Microsoft::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        if (m_rtCamera.ProcessChessboardImage())
        {
            SelectedFrame(m_rtCamera.IntrinsicCalibrationFrames().GetAt(m_rtCamera.IntrinsicCalibrationFrames().Size() - 1));

            IsPrimaryButtonEnabled(m_rtCamera.IntrinsicCalibrationFrames().Size() >= 4);
        }
    }

    void IntrinsicCalibrationDialog::BackToCamera_Click(Windows::Foundation::IInspectable const& /*sender*/, Microsoft::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        SelectedFrame(nullptr);
    }

    void IntrinsicCalibrationDialog::ContentDialog_PrimaryButtonClick(Microsoft::UI::Xaml::Controls::ContentDialog const& /*sender*/, Microsoft::UI::Xaml::Controls::ContentDialogButtonClickEventArgs const& /*e*/)
    {
        m_rtCamera.IntrinsicCalibrate();
    }

}
