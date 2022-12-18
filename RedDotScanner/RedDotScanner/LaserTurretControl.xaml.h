#pragma once

#include "winrt/Microsoft.UI.Xaml.h"
#include "winrt/Microsoft.UI.Xaml.Markup.h"
#include "winrt/Microsoft.UI.Xaml.Controls.Primitives.h"
#include "LaserTurretControl.g.h"


namespace winrt::RedDotScanner::implementation
{
    struct LaserTurretControl : LaserTurretControlT<LaserTurretControl>
    {
        LaserTurretControl();

        winrt::RedDotScanner::LaserTurret LaserTurret();
        void LaserTurret(winrt::RedDotScanner::LaserTurret const& value);

        void MovementButton_PointerPressed(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
        void MovementButton_PointerReleased(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
        void MovementButton_KeyDown(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::Input::KeyRoutedEventArgs const& e);
        void MovementButton_KeyUp(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::Input::KeyRoutedEventArgs const& e);

    private:
        RedDotScanner::LaserTurret m_rtLaserTurret { nullptr };

        static const uint8_t s_laserPanSpeed;
        static const uint8_t s_laserTiltSpeed;
    };
}

namespace winrt::RedDotScanner::factory_implementation
{
    struct LaserTurretControl : LaserTurretControlT<LaserTurretControl, implementation::LaserTurretControl>
    {
    };
}
