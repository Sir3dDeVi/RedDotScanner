#pragma once

#include "App.xaml.g.h"
#include "WorldView.h"

namespace winrt::RedDotScanner::implementation
{
    struct App : AppT<App>
    {
        App();

        void OnLaunched(Microsoft::UI::Xaml::LaunchActivatedEventArgs const&);

    private:
        winrt::Microsoft::UI::Xaml::Window window{ nullptr };
    };
}
