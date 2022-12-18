#pragma once

#include "winrt/Microsoft.UI.Xaml.h"
#include "winrt/Microsoft.UI.Xaml.Markup.h"
#include "winrt/Microsoft.UI.Xaml.Controls.Primitives.h"
#include "IntrinsicCalibrationDialog.g.h"

#include "Helpers/BoolToVisibilityConverter.h"
#include "ViewModel/NotifyPropretyChangedImpl.h"


namespace winrt::RedDotScanner::implementation
{
    struct IntrinsicCalibrationDialog
        : IntrinsicCalibrationDialogT<IntrinsicCalibrationDialog>
        , NotifyPropertyChangedImplT<IntrinsicCalibrationDialog>
    {
        IntrinsicCalibrationDialog(winrt::Microsoft::UI::Xaml::XamlRoot const& rtXamlRoot, winrt::RedDotScanner::Camera const& rtCamera);

    public:
        winrt::RedDotScanner::Camera Camera();

        Microsoft::UI::Xaml::Media::Imaging::WriteableBitmap PreviewFrame();
        bool IsCameraMode();
        bool IsPictureMode();

        Microsoft::UI::Xaml::Media::Imaging::WriteableBitmap SelectedFrame();
        void SelectedFrame(Microsoft::UI::Xaml::Media::Imaging::WriteableBitmap const& value);

    public:        
        void CaptureFrame_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void BackToCamera_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ContentDialog_PrimaryButtonClick(Microsoft::UI::Xaml::Controls::ContentDialog const& sender, Microsoft::UI::Xaml::Controls::ContentDialogButtonClickEventArgs const& e);

    private:
        RedDotScanner::Camera m_rtCamera;

        Microsoft::UI::Xaml::Media::Imaging::WriteableBitmap m_rtSelectedFrame { nullptr };
    };
}

namespace winrt::RedDotScanner::factory_implementation
{
    struct IntrinsicCalibrationDialog : IntrinsicCalibrationDialogT<IntrinsicCalibrationDialog, implementation::IntrinsicCalibrationDialog>
    {
    };
}
