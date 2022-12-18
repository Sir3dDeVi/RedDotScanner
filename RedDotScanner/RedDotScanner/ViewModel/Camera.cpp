#include "pch.h"
#include "Camera.h"
#include "Camera.g.cpp"

#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>


namespace winrt::RedDotScanner::implementation
{
    Camera::Camera(winrt::Microsoft::UI::Dispatching::DispatcherQueue const& rtDispatcherQueue, std::shared_ptr<scanner::Camera> camera)
        : m_rtDispatcherQueue(rtDispatcherQueue)
        , m_Camera(camera)
        , m_FrameWidth(m_Camera->getDevice()->getFrameWidth())
        , m_FrameHeight(m_Camera->getDevice()->getFrameHeight())
        , m_rtPreviewType(RedDotScanner::PreviewType::Color)
        , m_rtPreviewFrame(m_FrameWidth, m_FrameHeight)

        , m_FPS(0.0)
        , m_ImgProcTime(0.0)

        , m_ChessboardX(9)
        , m_ChessboardY(6)
        , m_rtIntrCalibFrames(winrt::single_threaded_observable_vector<Microsoft::UI::Xaml::Media::Imaging::WriteableBitmap>())

        , m_rtExtrCalibPoints(winrt::single_threaded_observable_vector<RedDotScanner::ExtrinsicCalibrationPoint>())
    {
        m_cvPreviewFrameBgra.create(cv::Size(m_FrameWidth, m_FrameHeight));

        m_Camera->addEventHandler(this);
    }


    hstring Camera::Name()
    {
        return L"PlayStation 3 Eye";
    }

    int32_t Camera::FrameWidth()
    {
        return m_FrameWidth;
    }

    int32_t Camera::FrameHeight()
    {
        return m_FrameHeight;
    }

    double Camera::FPS()
    {
        std::lock_guard lock(m_Mutex);

        return m_FPS;
    }

    double Camera::ImageProcessingTimeMs()
    {
        return m_ImgProcTime;
    }

    winrt::RedDotScanner::PreviewType Camera::PreviewType()
    {
        std::lock_guard lock(m_Mutex);

        return m_rtPreviewType;
    }

    void Camera::PreviewType(winrt::RedDotScanner::PreviewType const& value)
    {
        {
            std::lock_guard lock(m_Mutex);

            m_rtPreviewType = value;
        }

        RaisePropertyChanged(L"PreviewType");
        RaisePropertyChanged(L"IsPreviewingColor");
        RaisePropertyChanged(L"IsPreviewingUndistorted");
        RaisePropertyChanged(L"IsPreviewingThresholded");
    }

    winrt::Microsoft::UI::Xaml::Media::Imaging::WriteableBitmap Camera::PreviewFrame()
    {
        return m_rtPreviewFrame;
    }

    bool Camera::IsPreviewingColor()
    {
        return m_rtPreviewType == RedDotScanner::PreviewType::Color;
    }

    bool Camera::IsPreviewingUndistorted()
    {
        return m_rtPreviewType == RedDotScanner::PreviewType::Undistorted;
    }

    bool Camera::IsPreviewingThresholded()
    {
        return m_rtPreviewType == RedDotScanner::PreviewType::Thresholded;
    }


    bool Camera::IsPointVisible()
    {
        std::lock_guard lock(m_Mutex);

        return m_Point.has_value();
    }

    Windows::Foundation::Point Camera::Point()
    {
        std::lock_guard lock(m_Mutex);

        cv::Point2f p = m_Point.value_or(cv::Point2f(-1.0f, -1.0f));
        return Windows::Foundation::Point(p.x, p.y);
    }


    uint8_t Camera::Exposure()
    {
        return m_Camera->getDevice()->getExposure();
    }

    void Camera::Exposure(uint8_t value)
    {
        m_Camera->getDevice()->setExposure(value);
        
        RaisePropertyChanged(L"Exposure");
    }

    uint8_t Camera::Threshold()
    {
        return m_Camera->getThreshold();
    }

    void Camera::Threshold(uint8_t value)
    {
        m_Camera->setThreshold(value);

        RaisePropertyChanged(L"Threshold");
    }


    int32_t Camera::ChessboardX()
    {
        return m_ChessboardX;
    }

    void Camera::ChessboardX(int32_t value)
    {
        m_ChessboardX = value;
        RaisePropertyChanged(L"ChessboardX");
    }

    int32_t Camera::ChessboardY()
    {
        return m_ChessboardY;
    }

    void Camera::ChessboardY(int32_t value)
    {
        m_ChessboardY = value;
        RaisePropertyChanged(L"ChessboardY");
    }

    bool Camera::ProcessChessboardImage()
    {
        // Convert the last image to BGR
        cv::Mat3b frameBgr;
        {
            std::lock_guard lock(m_Mutex);

            cv::cvtColor(m_cvPreviewFrameBgra, frameBgr, cv::COLOR_BGRA2BGR);
        }

        const cv::Size2i chessboard(m_ChessboardX, m_ChessboardY);

        // Find chessboard
        std::vector<cv::Vec2f> imagePoints;
        const bool bFound = scanner::IntrinsicParameters::FindChessboard(frameBgr, chessboard, imagePoints);
        if (!bFound)
            return false;

        // Store the object and image points
        std::vector<cv::Vec3f> objectPoints;
        objectPoints.reserve(m_ChessboardX * m_ChessboardY);
        for (int y = 0; y < m_ChessboardY; ++y) {
            for (int x = 0; x < m_ChessboardX; ++x) {
                objectPoints.push_back(cv::Vec3f(static_cast<float>(x), static_cast<float>(y), 0.0f));
            }
        }

        m_IntrImagePoints.push_back(imagePoints);
        m_IntrObjectPoints.push_back(objectPoints);

        // Create preview image for the UI
        cv::drawChessboardCorners(frameBgr, chessboard, imagePoints, true);
        cv::Mat4b previewBgra;
        cv::cvtColor(frameBgr, previewBgra, cv::COLOR_BGR2BGRA);

        Microsoft::UI::Xaml::Media::Imaging::WriteableBitmap rtBitmap(m_FrameWidth, m_FrameHeight);
        memcpy(rtBitmap.PixelBuffer().data(), previewBgra.data, m_FrameWidth * m_FrameHeight * 4);
        m_rtIntrCalibFrames.Append(rtBitmap);

        return true;
    }

    winrt::Windows::Foundation::Collections::IObservableVector<winrt::Microsoft::UI::Xaml::Media::Imaging::WriteableBitmap> Camera::IntrinsicCalibrationFrames()
    {
        return m_rtIntrCalibFrames;
    }

    void Camera::IntrinsicCalibrate()
    {
        auto intr = scanner::IntrinsicParameters::Calibrate(
            cv::Size2i(m_FrameWidth, m_FrameHeight),
            m_IntrObjectPoints,
            m_IntrImagePoints
        );

        m_Camera->setIntrinsicParams(intr);
    }


    winrt::Windows::Foundation::Collections::IObservableVector<winrt::RedDotScanner::ExtrinsicCalibrationPoint> Camera::ExtrinsicCalibrationPoints()
    {
        return m_rtExtrCalibPoints;
    }

    void Camera::ExtrinsicCalibrate()
    {
        std::vector<cv::Point3f> objectPoints;
        std::vector<cv::Point2f> imagePoints;

        for (auto ecp : m_rtExtrCalibPoints) {
            objectPoints.emplace_back(ecp.WorldX(), ecp.WorldY(), ecp.WorldZ());
            imagePoints.emplace_back(ecp.ImageX(), ecp.ImageY());
        }

        scanner::ExtrinsicParameters extr = scanner::ExtrinsicParameters::Calibrate(
            m_Camera->getIntrinsicParams(),
            objectPoints,
            imagePoints
        );

        m_Camera->setExtrinsicParams(extr);
    }


    void Camera::Close()
    {
        m_Camera->removeEventHandler(this);

        m_Camera.reset();
    }

    void Camera::onFrame(const scanner::Camera* /*pCamera*/, const cv::Mat& colorBgr, const cv::Mat& undistortedBgr, const cv::Mat& thresholdedGray, const std::optional<cv::Point2f>& point, const std::chrono::high_resolution_clock::duration& imgProcTime)
    {
        // Calculate frame rate
        auto now = std::chrono::high_resolution_clock::now();
        auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_LastFrameTime);
        double fps = 1000.0 / dt.count();
        m_LastFrameTime = now;

        // Color conversion and modify UI stuff safely
        {
            std::lock_guard lock(m_Mutex);

            switch (m_rtPreviewType) {
                case RedDotScanner::PreviewType::Color:
                    cv::cvtColor(colorBgr, m_cvPreviewFrameBgra, cv::COLOR_BGR2BGRA);
                    break;
                case RedDotScanner::PreviewType::Undistorted:
                    cv::cvtColor(undistortedBgr, m_cvPreviewFrameBgra, cv::COLOR_BGR2BGRA);
                    break;
                case RedDotScanner::PreviewType::Thresholded:
                    cv::cvtColor(thresholdedGray, m_cvPreviewFrameBgra, cv::COLOR_GRAY2BGRA);
                    break;
            }

            m_FPS = fps;
            m_ImgProcTime = std::chrono::duration<double, std::milli>(imgProcTime).count();
            m_Point = point;
        }

        m_rtDispatcherQueue.TryEnqueue(
            Microsoft::UI::Dispatching::DispatcherQueuePriority::Normal,
            [this]
            {
                {
                    std::lock_guard lock(m_Mutex);

                    uint8_t* pPixels = m_rtPreviewFrame.PixelBuffer().data();
                    memcpy(pPixels, m_cvPreviewFrameBgra.data, m_FrameWidth * m_FrameHeight * 4 /* BGRA */);
                }

                m_rtPreviewFrame.Invalidate();
                RaisePropertyChanged(L"FPS");
                RaisePropertyChanged(L"ImageProcessingTimeMs");
                RaisePropertyChanged(L"IsPointVisible");
                RaisePropertyChanged(L"Point");
            }
        );
    }

}
