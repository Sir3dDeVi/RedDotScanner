#pragma once

#include "MainWindow.g.h"

#include "ViewModel/MainViewModel.h"
#include "ViewModel/NotifyPropretyChangedImpl.h"


namespace winrt::RedDotScanner::implementation
{
    struct MainWindow
        : MainWindowT<MainWindow>
        , NotifyPropertyChangedImplT<MainWindow>
    {
        MainWindow();

        Windows::Foundation::IInspectable CurrentTab();
        void CurrentTab(winrt::Windows::Foundation::IInspectable const& value);

        Microsoft::UI::Xaml::Visibility SetupViewVisibility();
        Microsoft::UI::Xaml::Visibility CaptureViewVisibility();

        winrt::RedDotScanner::MainViewModel ViewModel();

        void NavBar_Loaded(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void NavBar_SelectionChanged(Microsoft::UI::Xaml::Controls::NavigationView const& sender, Microsoft::UI::Xaml::Controls::NavigationViewSelectionChangedEventArgs const& args);
        Windows::Foundation::IAsyncAction SaveAs_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& e);
        Windows::Foundation::IAsyncAction Open_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void Window_Closed(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::WindowEventArgs const& args);
    private:
        bool m_bSetupView;

        RedDotScanner::MainViewModel m_rtMainViewModel { nullptr };

        HWND GetMyHWND();
        void ResizeWindow(int width, int height);
    };
}

namespace winrt::RedDotScanner::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
