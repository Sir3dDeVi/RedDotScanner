#pragma once
#include "Triangulator.g.h"

#include "ViewModel/NotifyPropretyChangedImpl.h"

#include "ViewModel/Camera.h"
#include "WorldView.h"

#include <memory>

#include <scanner/Triangulator.h>


namespace winrt::RedDotScanner::implementation
{
    struct Triangulator
        : TriangulatorT<Triangulator>
        , NotifyPropertyChangedImplT<Triangulator>
        , scanner::ITriangulatorCallback
    {
        Triangulator(winrt::Microsoft::UI::Dispatching::DispatcherQueue const& rtDispatcherQueue, RedDotScanner::WorldView const& rtWorldView);

        void RefreshCameras();
        winrt::Windows::Foundation::Collections::IObservableVector<winrt::RedDotScanner::Camera> Cameras();

        hstring PointXString();
        hstring PointYString();
        hstring PointZString();

        // IClosable
        void Close();

    public:
        scanner::Triangulator& getTriangulator();
        void clearRTCameras();
        void triangulatorUpdated();

    public: // ITriangulatorCallback
        virtual void onTriangulatorResult(const std::optional<cv::Point3f>& point, const std::map<const scanner::Camera*, std::optional<scanner::RayInfo>>& rays) override;

    private:
        std::unique_ptr<scanner::Triangulator> m_Triangulator; // this shouldn't be a unique_ptr, but the ViewModels are not destructed, so yeah...

        Microsoft::UI::Dispatching::DispatcherQueue m_rtDispatcherQueue;
        Windows::Foundation::Collections::IObservableVector<RedDotScanner::Camera> m_rtCameras;

        WorldView* m_pWorldView;

        std::optional<cv::Point3f> m_Point;
    };
}
namespace winrt::RedDotScanner::factory_implementation
{
    struct Triangulator : TriangulatorT<Triangulator, implementation::Triangulator>
    {
    };
}