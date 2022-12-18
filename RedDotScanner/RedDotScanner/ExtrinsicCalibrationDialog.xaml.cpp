#include "pch.h"
#include "ExtrinsicCalibrationDialog.xaml.h"
#if __has_include("ExtrinsicCalibrationDialog.g.cpp")
#include "ExtrinsicCalibrationDialog.g.cpp"
#endif

#include <cmath>


using namespace winrt;
using namespace Microsoft::UI::Xaml;


namespace winrt::RedDotScanner::implementation
{
    const float ExtrinsicCalibrationDialog::s_PointRadius = 5.0f;

    ExtrinsicCalibrationDialog::ExtrinsicCalibrationDialog(winrt::Microsoft::UI::Xaml::XamlRoot const& rtXamlRoot, winrt::RedDotScanner::Camera const& rtCamera)
        : m_rtCamera(rtCamera)
        , m_bMouseDown(false)
    {
        InitializeComponent();

        this->XamlRoot(rtXamlRoot);

        m_rtCamera.ExtrinsicCalibrationPoints().Clear();
    }

    winrt::RedDotScanner::Camera ExtrinsicCalibrationDialog::Camera()
    {
        return m_rtCamera;
    }

    winrt::RedDotScanner::ExtrinsicCalibrationPoint ExtrinsicCalibrationDialog::SelectedPoint()
    {
        return m_rtSelectedPoint;
    }

    bool ExtrinsicCalibrationDialog::IsPointSelected()
    {
        return m_rtSelectedPoint != nullptr;
    }

    bool ExtrinsicCalibrationDialog::CanCalibrate()
    {
        return m_rtCamera.ExtrinsicCalibrationPoints().Size() >= 4;
    }

    winrt::Windows::Globalization::NumberFormatting::DecimalFormatter ExtrinsicCalibrationDialog::CoordinateFormatter()
    {
        // I should be able to do this in XAML...

        Windows::Globalization::NumberFormatting::IncrementNumberRounder rounder;
        rounder.Increment(0.001);

        Windows::Globalization::NumberFormatting::DecimalFormatter formatter;
        formatter.FractionDigits(3);
        formatter.IsDecimalPointAlwaysDisplayed(true);
        formatter.NumberRounder(rounder);

        return formatter;
    }

    void ExtrinsicCalibrationDialog::Canvas_Draw(Microsoft::Graphics::Canvas::UI::Xaml::CanvasControl const& /*sender*/, Microsoft::Graphics::Canvas::UI::Xaml::CanvasDrawEventArgs const& args)
    {
        auto g = args.DrawingSession();

        g.Clear(Windows::UI::ColorHelper::FromArgb(0, 0, 0, 0));

        for (auto ecp : m_rtCamera.ExtrinsicCalibrationPoints()) {
            const bool bSelected = ecp == m_rtSelectedPoint;

            Windows::UI::Color color = bSelected ? Windows::UI::ColorHelper::FromArgb(255, 255, 0, 0) : Windows::UI::ColorHelper::FromArgb(255, 255, 255, 255);

            g.FillCircle(ecp.ImageX(), ecp.ImageY(), s_PointRadius, color);
            g.DrawCircle(ecp.ImageX(), ecp.ImageY(), s_PointRadius, Windows::UI::ColorHelper::FromArgb(255, 0, 0, 0), 1.0f);
        }
    }

    void ExtrinsicCalibrationDialog::Canvas_PointerMoved(Windows::Foundation::IInspectable const& /*sender*/, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e)
    {
        if (m_bMouseDown) {
            assert(m_rtSelectedPoint != nullptr);

            auto p = e.GetCurrentPoint(Canvas()).Position();

            m_rtSelectedPoint.ImageX(p.X);
            m_rtSelectedPoint.ImageY(p.Y);

            Canvas().Invalidate();
        }
    }

    void ExtrinsicCalibrationDialog::Canvas_PointerPressed(Windows::Foundation::IInspectable const& /*sender*/, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e)
    {
        m_bMouseDown = true;

        auto p = e.GetCurrentPoint(Canvas()).Position();

        float minDist = 10000.0f;
        m_rtSelectedPoint = nullptr;
        
        // Try to select point by mouse
        for (auto ecp : m_rtCamera.ExtrinsicCalibrationPoints()) {
            float dist = sqrtf(
                (p.X - ecp.ImageX()) * (p.X - ecp.ImageX()) +
                (p.Y - ecp.ImageY()) * (p.Y - ecp.ImageY())
            );
            
            if (dist < s_PointRadius && dist < minDist) {
                m_rtSelectedPoint = ecp;
                minDist = dist;
            }
        }

        if (!m_rtSelectedPoint) {
            RedDotScanner::ExtrinsicCalibrationPoint ecp;
            ecp.ImageX(p.X);
            ecp.ImageY(p.Y);

            m_rtCamera.ExtrinsicCalibrationPoints().Append(ecp);
            m_rtSelectedPoint = ecp;
        }

        // Refresh everything
        Canvas().Invalidate();
        RaisePropertyChanged(L"SelectedPoint");
        RaisePropertyChanged(L"IsPointSelected");

        RaisePropertyChanged(L"CanCalibrate");
    }

    void ExtrinsicCalibrationDialog::Canvas_PointerReleased(Windows::Foundation::IInspectable const& /*sender*/, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& /*e*/)
    {
        m_bMouseDown = false;
    }

    void ExtrinsicCalibrationDialog::DeleteCalibrationPoint_Click(Windows::Foundation::IInspectable const& /*sender*/, Microsoft::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        assert(m_rtSelectedPoint != nullptr);

        auto ecps = m_rtCamera.ExtrinsicCalibrationPoints();

        for (uint32_t i = 0;  i < ecps.Size(); ++i) {
            if (ecps.GetAt(i) == m_rtSelectedPoint) {
                ecps.RemoveAt(i);
                m_rtSelectedPoint = nullptr;

                Canvas().Invalidate();
                RaisePropertyChanged(L"SelectedPoint");
                RaisePropertyChanged(L"IsPointSelected");

                RaisePropertyChanged(L"CanCalibrate");
                return;
            }
        }
    }

    void ExtrinsicCalibrationDialog::ContentDialog_PrimaryButtonClick(Microsoft::UI::Xaml::Controls::ContentDialog const& /*sender*/, Microsoft::UI::Xaml::Controls::ContentDialogButtonClickEventArgs const& /*e*/)
    {
        m_rtCamera.ExtrinsicCalibrate();
    }

}
