#pragma once
#include "MainViewModel.g.h"
#include "NotifyPropretyChangedImpl.h"
#include "Scanner.h"


namespace winrt::RedDotScanner::implementation
{
    struct MainViewModel
        : MainViewModelT<MainViewModel>
        , NotifyPropertyChangedImplT<MainViewModel>
    {
        MainViewModel(winrt::Microsoft::UI::Dispatching::DispatcherQueue const& rtDispatcherQueue, RedDotScanner::WorldView const& rtWorldView);

        winrt::RedDotScanner::Scanner Scanner();

        // IClosable
        void Close();

    private:
        RedDotScanner::Scanner m_rtScanner;
    };
}
namespace winrt::RedDotScanner::factory_implementation
{
    struct MainViewModel : MainViewModelT<MainViewModel, implementation::MainViewModel>
    {
    };
}