#include "pch.h"
#include "Triangulator.h"
#include "Triangulator.g.cpp"

#include <iomanip>

#include <scanner/PS3EyeCamera.h>
#include <Utils.h>


namespace winrt::RedDotScanner::implementation
{
    Triangulator::Triangulator(winrt::Microsoft::UI::Dispatching::DispatcherQueue const& rtDispatcherQueue, RedDotScanner::WorldView const& rtWorldView)
        : m_Triangulator(std::make_unique<scanner::Triangulator>())
        , m_rtDispatcherQueue(rtDispatcherQueue)
        , m_rtCameras(winrt::single_threaded_observable_vector<RedDotScanner::Camera>())
        , m_pWorldView(winrt::get_self<WorldView>(rtWorldView))
    {
        m_Triangulator->addEventHandler(this);

        RefreshCameras();
    }

    void Triangulator::Close()
    {
        m_Triangulator->removeEventHandler(this);

        clearRTCameras();

        m_Triangulator.reset();
    }

    void Triangulator::RefreshCameras()
    {
        // Clear cameras
        auto cameras = m_Triangulator->getCameras();
        for (auto nCamera : cameras)
            m_Triangulator->removeCamera(nCamera);

        m_rtCameras.Clear();

        // Create cameras
        scanner::PS3EyeCameraFactory ps3EyeFactory;
        const auto ps3Eyes = ps3EyeFactory.getDevices();

        for (auto di : ps3Eyes) {
            auto ps3eye = ps3EyeFactory.create(di.usb);
            auto camera = m_Triangulator->addCamera(std::move(ps3eye));
            winrt::RedDotScanner::Camera rtCamera = winrt::make<winrt::RedDotScanner::implementation::Camera>(m_rtDispatcherQueue, camera);
            m_rtCameras.Append(rtCamera);
        }

        RaisePropertyChanged(L"Cameras");
    }

    winrt::Windows::Foundation::Collections::IObservableVector<winrt::RedDotScanner::Camera> Triangulator::Cameras()
    {
        return m_rtCameras;
    }

    hstring Triangulator::PointXString()
    {
        if (m_Point)
            return winrt::to_hstring(static_cast<std::string>(utils::S() << std::fixed << std::setprecision(3) << m_Point->x));
        else
            return L"-";
    }
    
    hstring Triangulator::PointYString()
    {
        if (m_Point)
            return winrt::to_hstring(static_cast<std::string>(utils::S() << std::fixed << std::setprecision(3) << m_Point->y));
        else
            return L"-";
    }

    hstring Triangulator::PointZString()
    {
        if (m_Point)
            return winrt::to_hstring(static_cast<std::string>(utils::S() << std::fixed << std::setprecision(3) << m_Point->z));
        else
            return L"-";
    }


    scanner::Triangulator& Triangulator::getTriangulator()
    {
        assert(m_Triangulator != nullptr);

        return *m_Triangulator.get();
    }

    void Triangulator::clearRTCameras()
    {
        std::optional<cv::Point3f> emptyPoint;
        std::map<const scanner::Camera*, std::optional<scanner::RayInfo>> emptyRays;
        m_pWorldView->UpdateFromTriangulator(emptyPoint, emptyRays);

        for (auto rtCamera : m_rtCameras)
            rtCamera.Close();
        m_rtCameras.Clear();

        RaisePropertyChanged(L"Cameras");
    }

    void Triangulator::triangulatorUpdated()
    {
        clearRTCameras();

        // Populate cameras from "native" type
        for (auto camera : m_Triangulator->getCameras()) {
            RedDotScanner::Camera rtCamera = winrt::make<RedDotScanner::implementation::Camera>(m_rtDispatcherQueue, camera);
            m_rtCameras.Append(rtCamera);
        }

        RaisePropertyChanged(L"Cameras");
    }


    void Triangulator::onTriangulatorResult(const std::optional<cv::Point3f>& point, const std::map<const scanner::Camera*, std::optional<scanner::RayInfo>>& rays)
    {
        m_rtDispatcherQueue.TryEnqueue(
            Microsoft::UI::Dispatching::DispatcherQueuePriority::Normal,
            [this, point, rays]
            {
                m_pWorldView->UpdateFromTriangulator(point, rays);

                m_Point = point;
                RaisePropertyChanged(L"PointXString");
                RaisePropertyChanged(L"PointYString");
                RaisePropertyChanged(L"PointZString");            }
        );
    }
}