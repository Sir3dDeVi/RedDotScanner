#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <chrono>
#include <list>
#include <mutex>
#include <optional>

#include "PS3EyeCamera.h"
#include "FrameGrabberThread.h"
#include "IntrinsicParameters.h"
#include "ExtrinsicParameters.h"
#include "EventSource.h"


namespace scanner
{

	class Camera;

	class ICameraCallback
	{
	public:
		virtual void onFrame(const Camera* pCamera, const cv::Mat& colorBgr, const cv::Mat& undistortedBgr, const cv::Mat& thresholdedGray, const std::optional<cv::Point2f>& point, const std::chrono::high_resolution_clock::duration& imgProcTime) = 0;
	};


	class Camera
		: public EventSourceT<ICameraCallback>
		, public IFrameGrabberCallback
	{
		static const double s_MinDotWeight;

		static unsigned int s_nCamera;

		std::shared_ptr<PS3EyeCamera> m_Device;
		unsigned int m_CameraId;
		FrameGrabberThread m_Thread;
		bool m_bThreadInited;

		mutable std::mutex m_Mutex;
		IntrinsicParameters m_IntrinsicParams;
		ExtrinsicParameters m_ExtrinsicParams;

		unsigned char m_Threshold;

		cv::Mat m_cvColor;

		cv::Mat m_cvUndistortMap1;
		cv::Mat m_cvUndistortMap2;
		cv::Mat m_cvUndistorted;

		cv::Mat m_cvGray;
		cv::Mat m_cvThresholded;
	
	public:
		Camera(PS3EyeCamera&& device);
		~Camera();

		std::shared_ptr<PS3EyeCamera> getDevice() const;

		const IntrinsicParameters& getIntrinsicParams() const;
		void setIntrinsicParams(const IntrinsicParameters& intrinsicParams);

		const ExtrinsicParameters& getExtrinsicParams() const;
		void setExtrinsicParams(const ExtrinsicParameters& extrinsicParams);

		unsigned char getThreshold() const;
		void setThreshold(unsigned char threshold);

	public: // IFrameGrabberCallback
		virtual void onFrame(const cv::Mat& frame) override;
	};

}

#endif // __CAMERA_H__