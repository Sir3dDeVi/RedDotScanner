#include "pch.h"
#include "MainViewModel.h"
#include "MainViewModel.g.cpp"


namespace winrt::RedDotScanner::implementation
{
    MainViewModel::MainViewModel(winrt::Microsoft::UI::Dispatching::DispatcherQueue const& rtDispatcherQueue, RedDotScanner::WorldView const& rtWorldView)
        : m_rtScanner(rtDispatcherQueue, rtWorldView)
    {
    }

    void MainViewModel::Close()
    {
        m_rtScanner.Close();
    }

    winrt::RedDotScanner::Scanner MainViewModel::Scanner()
    {
        return m_rtScanner;
    }
}
