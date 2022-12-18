#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;


namespace winrt::RedDotScanner::implementation
{
    MainWindow::MainWindow()
        : m_bSetupView(true)
    {
        InitializeComponent();

        m_rtMainViewModel = winrt::make<MainViewModel>(DispatcherQueue(), CaptureView().WorldView()); // TODO: this is a bit ugly

        ResizeWindow(1680, 945);

        ExtendsContentIntoTitleBar(true);
        SetTitleBar(AppTitleBar());
    }

    Windows::Foundation::IInspectable MainWindow::CurrentTab()
    {
        if (m_bSetupView)
            return NavItemSetup();
        else
            return NavItemCapture();
    }

    void MainWindow::CurrentTab(winrt::Windows::Foundation::IInspectable const& value)
    {
        if (value == NavItemSetup())
            m_bSetupView = true;
        else if (value == NavItemCapture())
            m_bSetupView = false;
        else
            assert(false);

        RaisePropertyChanged(L"CurrentTab");
        RaisePropertyChanged(L"SetupViewVisibility");
        RaisePropertyChanged(L"CaptureViewVisibility");
    }

    Microsoft::UI::Xaml::Visibility MainWindow::SetupViewVisibility()
    {
        // Why can't things just work with WinUI 3???
        // https://github.com/microsoft/microsoft-ui-xaml/issues/4966
        
        return m_bSetupView ? Microsoft::UI::Xaml::Visibility::Visible : Microsoft::UI::Xaml::Visibility::Collapsed;
    }

    Microsoft::UI::Xaml::Visibility MainWindow::CaptureViewVisibility()
    {
        return m_bSetupView ? Microsoft::UI::Xaml::Visibility::Collapsed : Microsoft::UI::Xaml::Visibility::Visible;
    }

    winrt::RedDotScanner::MainViewModel MainWindow::ViewModel()
    {
        return m_rtMainViewModel;
    }

    Windows::Foundation::IAsyncAction MainWindow::SaveAs_Click(Windows::Foundation::IInspectable const& /*sender*/, Microsoft::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        // https://learn.microsoft.com/en-us/windows/apps/develop/ui-input/display-ui-objects#winui-3-with-c
        winrt::Windows::Storage::Pickers::FileSavePicker rtSaveDialog;
        auto rtIntializeWithWindow{ rtSaveDialog.as<::IInitializeWithWindow>() };
        rtIntializeWithWindow->Initialize(GetMyHWND());
        rtSaveDialog.FileTypeChoices().Insert(L"JSON File", winrt::single_threaded_vector<hstring>({ L".json" }));
        rtSaveDialog.SuggestedFileName(L"Untitled Scan");

        auto file = co_await rtSaveDialog.PickSaveFileAsync();
        if (file != nullptr) {
            m_rtMainViewModel.Scanner().Save(file.Path());
        }
    }

    Windows::Foundation::IAsyncAction MainWindow::Open_Click(Windows::Foundation::IInspectable const& /*sender*/, Microsoft::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        // https://learn.microsoft.com/en-us/windows/apps/develop/ui-input/display-ui-objects#winui-3-with-c
        winrt::Windows::Storage::Pickers::FileOpenPicker rtOpenDialog;
        auto rtIntializeWithWindow{ rtOpenDialog.as<::IInitializeWithWindow>() };
        rtIntializeWithWindow->Initialize(GetMyHWND());
        rtOpenDialog.FileTypeFilter().Append(L".json");

        auto file = co_await rtOpenDialog.PickSingleFileAsync();
        if (file != nullptr) {
            m_rtMainViewModel.Scanner().Load(file.Path());
        }
    }

    void MainWindow::Window_Closed(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::WindowEventArgs const&)
    {
        // Release ViewModel (and it's "normal native" types) explicitly.
        m_rtMainViewModel.Close();
    }

    HWND MainWindow::GetMyHWND()
    {
        // Retrieve the window handle (HWND) of the current WinUI 3 window.
        auto windowNative{ this->try_as<::IWindowNative>() };
        winrt::check_bool(windowNative);
        HWND hWnd{ 0 };
        windowNative->get_WindowHandle(&hWnd);

        assert(hWnd != 0);

        return hWnd;
    }

    void MainWindow::ResizeWindow(int width, int height)
    {
        // https://learn.microsoft.com/en-us/windows/apps/windows-app-sdk/windowing/windowing-overview

        HWND hWnd = GetMyHWND();

        if (hWnd != 0) {
            float dpi = static_cast<float>(::GetDpiForWindow(hWnd));

            ::SetWindowPos(
                hWnd, 0,
                0, 0,
                static_cast<int>(width * dpi / 96.0f), static_cast<int>(height * dpi / 96.0f),
                SWP_NOMOVE | SWP_NOZORDER
            );
        }
    }
}
