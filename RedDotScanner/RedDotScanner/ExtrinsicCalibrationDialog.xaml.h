#pragma once

#include "winrt/Microsoft.UI.Xaml.h"
#include "winrt/Microsoft.UI.Xaml.Markup.h"
#include "winrt/Microsoft.UI.Xaml.Controls.Primitives.h"
#include "winrt/Windows.Globalization.NumberFormatting.h"
#include "ExtrinsicCalibrationDialog.g.h"

#include "ViewModel/NotifyPropretyChangedImpl.h"

namespace winrt::RedDotScanner::implementation
{
    struct ExtrinsicCalibrationDialog
        : ExtrinsicCalibrationDialogT<ExtrinsicCalibrationDialog>
        , NotifyPropertyChangedImplT<ExtrinsicCalibrationDialog>
    {
        ExtrinsicCalibrationDialog(winrt::Microsoft::UI::Xaml::XamlRoot const& rtXamlRoot, winrt::RedDotScanner::Camera const& rtCamera);

        winrt::RedDotScanner::Camera Camera();

        winrt::RedDotScanner::ExtrinsicCalibrationPoint SelectedPoint();
        bool IsPointSelected();
        bool CanCalibrate();

        winrt::Windows::Globalization::NumberFormatting::DecimalFormatter CoordinateFormatter();

    public:
        void Canvas_Draw(Microsoft::Graphics::Canvas::UI::Xaml::CanvasControl const& sender, Microsoft::Graphics::Canvas::UI::Xaml::CanvasDrawEventArgs const& args);
        void Canvas_PointerMoved(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
        void Canvas_PointerPressed(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
        void Canvas_PointerReleased(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);

        void DeleteCalibrationPoint_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& e);

        void ContentDialog_PrimaryButtonClick(Microsoft::UI::Xaml::Controls::ContentDialog const& sender, Microsoft::UI::Xaml::Controls::ContentDialogButtonClickEventArgs const& e);

    private:
        RedDotScanner::Camera m_rtCamera;
        RedDotScanner::ExtrinsicCalibrationPoint m_rtSelectedPoint { nullptr };

        bool m_bMouseDown;

        static const float s_PointRadius;
    };
}

namespace winrt::RedDotScanner::factory_implementation
{
    struct ExtrinsicCalibrationDialog : ExtrinsicCalibrationDialogT<ExtrinsicCalibrationDialog, implementation::ExtrinsicCalibrationDialog>
    {
    };
}
