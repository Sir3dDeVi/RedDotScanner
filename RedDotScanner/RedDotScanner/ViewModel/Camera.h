#pragma once
#include "Camera.g.h"
#include "winrt/RedDotScanner.h" // PreviewType enum
#include "ExtrinsicCalibrationPoint.h"

#include "NotifyPropretyChangedImpl.h"

#include <chrono>
#include <memory>
#include <mutex>
#include <vector>

#include <scanner/Camera.h>


namespace winrt::RedDotScanner::implementation
{
    struct Camera
        : CameraT<Camera>
        , NotifyPropertyChangedImplT<Camera>
        , scanner::ICameraCallback
    {
        Camera(winrt::Microsoft::UI::Dispatching::DispatcherQueue const& rtDispatcherQueue, std::shared_ptr<scanner::Camera> camera);

        // General info
        hstring Name();
        int32_t FrameWidth();
        int32_t FrameHeight();
        double FPS();
        double ImageProcessingTimeMs();

        // Previewimng
        winrt::RedDotScanner::PreviewType PreviewType();
        void PreviewType(winrt::RedDotScanner::PreviewType const& value);

        winrt::Microsoft::UI::Xaml::Media::Imaging::WriteableBitmap PreviewFrame();

        bool IsPreviewingColor();
        bool IsPreviewingUndistorted();
        bool IsPreviewingThresholded();

        // Point
        bool IsPointVisible();
        Windows::Foundation::Point Point();

        // Parameter(s)
        uint8_t Exposure();
        void Exposure(uint8_t value);

        uint8_t Threshold();
        void Threshold(uint8_t value);

        // Intrinsic calibration
        int32_t ChessboardX();
        void ChessboardX(int32_t value);
        int32_t ChessboardY();
        void ChessboardY(int32_t value);
        bool ProcessChessboardImage();
        winrt::Windows::Foundation::Collections::IObservableVector<winrt::Microsoft::UI::Xaml::Media::Imaging::WriteableBitmap> IntrinsicCalibrationFrames();
        void IntrinsicCalibrate();

        // Extrinsic calibration
        winrt::Windows::Foundation::Collections::IObservableVector<winrt::RedDotScanner::ExtrinsicCalibrationPoint> ExtrinsicCalibrationPoints();
        void ExtrinsicCalibrate();

        // IClosable
        void Close();

        // ICameraCallback
        virtual void onFrame(const scanner::Camera* pCamera, const cv::Mat& colorBgr, const cv::Mat& undistortedBgr, const cv::Mat& thresholdedGray, const std::optional<cv::Point2f>& point, const std::chrono::high_resolution_clock::duration& imgProcTime) override;

    private:
        Microsoft::UI::Dispatching::DispatcherQueue m_rtDispatcherQueue;

        std::shared_ptr<scanner::Camera> m_Camera;
        const int m_FrameWidth, m_FrameHeight;

        std::mutex m_Mutex;
        winrt::RedDotScanner::PreviewType m_rtPreviewType;
        cv::Mat4b m_cvPreviewFrameBgra;
        winrt::Microsoft::UI::Xaml::Media::Imaging::WriteableBitmap m_rtPreviewFrame;
        std::chrono::high_resolution_clock::time_point m_LastFrameTime;
        double m_FPS, m_ImgProcTime;
        std::optional<cv::Point2f> m_Point;

        // Intrinsic calibration
        int32_t m_ChessboardX;
        int32_t m_ChessboardY;
        std::vector<std::vector<cv::Vec3f>> m_IntrObjectPoints;
        std::vector<std::vector<cv::Vec2f>> m_IntrImagePoints;
        Windows::Foundation::Collections::IObservableVector<Microsoft::UI::Xaml::Media::Imaging::WriteableBitmap> m_rtIntrCalibFrames;

        // Extrinsic calibration
        Windows::Foundation::Collections::IObservableVector<RedDotScanner::ExtrinsicCalibrationPoint> m_rtExtrCalibPoints;
    };
}
