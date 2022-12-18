#include "Camera.h"
#include "Utils.h"

#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>


namespace scanner
{


const double Camera::s_MinDotWeight = 5.0;

unsigned int Camera::s_nCamera = 0;


Camera::Camera(PS3EyeCamera&& device)
	: m_Device(std::make_shared<PS3EyeCamera>(std::move(device)))
	, m_CameraId(++s_nCamera)
	, m_Thread(m_Device, this)
	, m_bThreadInited(false)
	, m_Threshold(128)

	, m_cvColor(cv::Size(m_Device->getFrameWidth(), m_Device->getFrameHeight()), CV_8UC3)
	, m_cvUndistorted(cv::Size(m_Device->getFrameWidth(), m_Device->getFrameHeight()), CV_8UC3)
	, m_cvGray(cv::Size(m_Device->getFrameWidth(), m_Device->getFrameHeight()), CV_8U)
	, m_cvThresholded(cv::Size(m_Device->getFrameWidth(), m_Device->getFrameHeight()), CV_8U)
{
	// Setup default parameters
	IntrinsicParameters intr;
	intr.cx = m_Device->getFrameWidth() / 2.0f;
	intr.cy = m_Device->getFrameHeight() / 2.0f;
	intr.fx = intr.fy = (m_Device->getFrameWidth() / 2.0f) / tanf(utils::deg2rad(30.0f)); // 60 deg HFOV

	setIntrinsicParams(intr);

	ExtrinsicParameters extr;
	extr.position = cv::Point3f(0.0f, 0.0f, 0.0f);
	extr.cameraToWorld = cv::Matx33f::eye();
}

Camera::~Camera()
{
	m_Thread.stop();

	--s_nCamera; // TODO: this is not OK this way, but it will work (for now)
}

std::shared_ptr<PS3EyeCamera> Camera::getDevice() const
{
	return m_Device;
}

const IntrinsicParameters& Camera::getIntrinsicParams() const
{
	std::lock_guard guard(m_Mutex);

	return m_IntrinsicParams;
}

void Camera::setIntrinsicParams(const IntrinsicParameters& intrinsicParams)
{
	std::lock_guard guard(m_Mutex);

	m_IntrinsicParams = intrinsicParams;

	cv::Mat_<double> I = cv::Mat_<double>::eye(3, 3);

	const auto& cameraMatrix = m_IntrinsicParams.getCameraMatrix();
	const auto& distortion = m_IntrinsicParams.getDistortionCoefficients();
	cv::initUndistortRectifyMap(
		cameraMatrix,
		distortion,
		I,
		cameraMatrix,
		cv::Size(m_Device->getFrameWidth(), m_Device->getFrameHeight()),
		CV_16SC2,
		m_cvUndistortMap1,
		m_cvUndistortMap2
	);
}


const ExtrinsicParameters& Camera::getExtrinsicParams() const
{
	std::lock_guard guard(m_Mutex);

	return m_ExtrinsicParams;
}

void Camera::setExtrinsicParams(const ExtrinsicParameters& extrinsicParams)
{
	std::lock_guard guard(m_Mutex);

	m_ExtrinsicParams = extrinsicParams;
}

unsigned char Camera::getThreshold() const
{
	return m_Threshold;
}

void Camera::setThreshold(unsigned char threshold)
{
	m_Threshold = threshold;
}

void Camera::onFrame(const cv::Mat& bayer)
{
	if (!m_bThreadInited) {
		DWORD_PTR mask = 1 << m_CameraId;
		auto ret = SetThreadAffinityMask(GetCurrentThread(), mask);

		m_bThreadInited = true;
	}

	// Do the image processing
	auto tStart = std::chrono::high_resolution_clock::now();

	cv::cvtColor(bayer, m_cvColor, cv::COLOR_BayerGB2BGR);

	{
		std::lock_guard guard(m_Mutex);

		//cv::undistort(m_cvColor, m_cvUndistorted, m_IntrinsicParams.getCameraMatrix(), m_IntrinsicParams.getDistortionCoefficients());
		cv::remap(m_cvColor, m_cvUndistorted, m_cvUndistortMap1, m_cvUndistortMap2, cv::INTER_LINEAR, cv::BORDER_CONSTANT);
	}

	cv::cvtColor(m_cvColor, m_cvGray, cv::COLOR_BGR2GRAY);

	cv::threshold(m_cvGray, m_cvThresholded, m_Threshold, 255, cv::THRESH_BINARY);

	cv::Moments m = cv::moments(m_cvThresholded, true);

	std::optional<cv::Point2f> point;
	if (m.m00 >= s_MinDotWeight) {
		point = cv::Point2f(static_cast<float>(m.m10 / m.m00), static_cast<float>(m.m01 / m.m00));
	}

	auto tEnd = std::chrono::high_resolution_clock::now();

	// Callback
	invoke(&ICameraCallback::onFrame, this, m_cvColor, m_cvUndistorted, m_cvThresholded, point, tEnd - tStart);
}

}