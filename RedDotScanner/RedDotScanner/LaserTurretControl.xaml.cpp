#include "pch.h"
#include "LaserTurretControl.xaml.h"
#if __has_include("LaserTurretControl.g.cpp")
#include "LaserTurretControl.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;


namespace winrt::RedDotScanner::implementation
{
    const uint8_t LaserTurretControl::s_laserPanSpeed = 50;
    const uint8_t LaserTurretControl::s_laserTiltSpeed = 30;


    LaserTurretControl::LaserTurretControl()
    {
        InitializeComponent();

        // Hook up event handlers this way to capture "mouse down" and "mouse up" events.
        // (Becaues the Button is "smart" and it only exposes the Click event...)
        Microsoft::UI::Xaml::Controls::Button movementButtons[] = { TiltUpButton(), TiltDownButton(), PanLeftButton(), PanRightButton() };
        for (auto& button : movementButtons) {
            button.AddHandler(
                Microsoft::UI::Xaml::UIElement::PointerPressedEvent(),
                winrt::box_value(Microsoft::UI::Xaml::Input::PointerEventHandler(this, &LaserTurretControl::MovementButton_PointerPressed)),
                true
            );
            button.AddHandler(
                Microsoft::UI::Xaml::UIElement::PointerReleasedEvent(),
                winrt::box_value(Microsoft::UI::Xaml::Input::PointerEventHandler(this, &LaserTurretControl::MovementButton_PointerReleased)),
                true
            );

            button.AddHandler(
                Microsoft::UI::Xaml::UIElement::KeyDownEvent(),
                winrt::box_value(Microsoft::UI::Xaml::Input::KeyEventHandler(this, &LaserTurretControl::MovementButton_KeyDown)),
                true
            );
            button.AddHandler(
                Microsoft::UI::Xaml::UIElement::KeyUpEvent(),
                winrt::box_value(Microsoft::UI::Xaml::Input::KeyEventHandler(this, &LaserTurretControl::MovementButton_KeyUp)),
                true
            );
        }
    }

    winrt::RedDotScanner::LaserTurret LaserTurretControl::LaserTurret()
    {
        return m_rtLaserTurret;
    }

    void LaserTurretControl::LaserTurret(winrt::RedDotScanner::LaserTurret const& value)
    {
        m_rtLaserTurret = value;
    }

    void LaserTurretControl::MovementButton_PointerPressed(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& /* e */)
    {
        auto senderButton = sender.as<Microsoft::UI::Xaml::Controls::Button>();

        if (senderButton == TiltUpButton())
            m_rtLaserTurret.TiltRotate(s_laserTiltSpeed);

        if (senderButton == TiltDownButton())
            m_rtLaserTurret.TiltRotate(-s_laserTiltSpeed);

        if (senderButton == PanLeftButton())
            m_rtLaserTurret.PanRotate(s_laserPanSpeed);

        if (senderButton == PanRightButton())
            m_rtLaserTurret.PanRotate(-s_laserPanSpeed);
    }

    void LaserTurretControl::MovementButton_PointerReleased(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& /* e */)
    {
        auto senderButton = sender.as<Microsoft::UI::Xaml::Controls::Button>();

        if (senderButton == TiltUpButton() || senderButton == TiltDownButton())
            m_rtLaserTurret.TiltRotate(0);

        if (senderButton == PanLeftButton() || senderButton == PanRightButton())
            m_rtLaserTurret.PanRotate(0);
    }

    void LaserTurretControl::MovementButton_KeyDown(Windows::Foundation::IInspectable const& /* sender */, Microsoft::UI::Xaml::Input::KeyRoutedEventArgs const& e)
    {
        if (e.Key() == Windows::System::VirtualKey::Up) {
            Microsoft::UI::Xaml::VisualStateManager::GoToState(TiltUpButton(), L"Pressed", true);
            m_rtLaserTurret.TiltRotate(s_laserTiltSpeed);
        }

        if (e.Key() == Windows::System::VirtualKey::Down) {
            Microsoft::UI::Xaml::VisualStateManager::GoToState(TiltDownButton(), L"Pressed", true);
            m_rtLaserTurret.TiltRotate(-s_laserTiltSpeed);
        }

        if (e.Key() == Windows::System::VirtualKey::Left) {
            Microsoft::UI::Xaml::VisualStateManager::GoToState(PanLeftButton(), L"Pressed", true);
            m_rtLaserTurret.PanRotate(s_laserPanSpeed);
        }

        if (e.Key() == Windows::System::VirtualKey::Right) {
            Microsoft::UI::Xaml::VisualStateManager::GoToState(PanRightButton(), L"Pressed", true);
            m_rtLaserTurret.PanRotate(-s_laserPanSpeed);
        }
    }

    void LaserTurretControl::MovementButton_KeyUp(Windows::Foundation::IInspectable const& /* sender */, Microsoft::UI::Xaml::Input::KeyRoutedEventArgs const& e)
    {
        if (e.Key() == Windows::System::VirtualKey::Up) {
            m_rtLaserTurret.TiltRotate(0);
            Microsoft::UI::Xaml::VisualStateManager::GoToState(TiltUpButton(), L"Normal", true);
        }

        if (e.Key() == Windows::System::VirtualKey::Down) {
            m_rtLaserTurret.TiltRotate(0);
            Microsoft::UI::Xaml::VisualStateManager::GoToState(TiltDownButton(), L"Normal", true);
        }

        if (e.Key() == Windows::System::VirtualKey::Left) {
            m_rtLaserTurret.PanRotate(0);
            Microsoft::UI::Xaml::VisualStateManager::GoToState(PanLeftButton(), L"Normal", true);
        }

        if (e.Key() == Windows::System::VirtualKey::Right) {
            m_rtLaserTurret.PanRotate(0);
            Microsoft::UI::Xaml::VisualStateManager::GoToState(PanRightButton(), L"Normal", true);
        }
    }
}
