#include "IntrinsicParameters.h"

#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>


namespace scanner
{

IntrinsicParameters::IntrinsicParameters()
	: cx(0.0f)
	, cy(0.0f)
	, fx(1.0f)
	, fy(1.0f)
{
	memset(distortionRadial, 0, sizeof(distortionRadial));
	memset(distortionPlanar, 0, sizeof(distortionPlanar));
}

cv::Mat1f IntrinsicParameters::getCameraMatrix() const
{
	cv::Mat1f cameraMatrix(cv::Size(3, 3), 0.0f);

	cameraMatrix(0, 0) = fx;
	cameraMatrix(1, 1) = fy;
	cameraMatrix(0, 2) = cx;
	cameraMatrix(1, 2) = cy;
	cameraMatrix(2, 2) = 1.0f;

	return cameraMatrix;
}

std::vector<float> IntrinsicParameters::getDistortionCoefficients() const
{
	std::vector<float> distCoeffs = {
		distortionRadial[0],
		distortionRadial[1],
		distortionPlanar[0],
		distortionPlanar[1],
		distortionRadial[2]
	};

	return distCoeffs;
}

cv::Point2f IntrinsicParameters::undistortPixel(const cv::Point2f& distortedPixel) const
{
	std::vector<cv::Point2f> distortedPixels{ distortedPixel };
	std::vector<cv::Point2f> undistortedPixels;

	cv::undistortImagePoints(distortedPixels, undistortedPixels, getCameraMatrix(), getDistortionCoefficients());

	return undistortedPixels[0];
}

bool IntrinsicParameters::FindChessboard(const cv::Mat3b& image, const cv::Size2i& chessboard, std::vector<cv::Vec2f>& cornerPoints)
{
	bool bFound = cv::findChessboardCorners(image, chessboard, cornerPoints, cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_NORMALIZE_IMAGE | cv::CALIB_CB_FAST_CHECK);

	if (bFound) {
		cv::Mat1b imageGray;
		cv::cvtColor(image, imageGray, cv::COLOR_BGR2GRAY);

		cv::cornerSubPix(imageGray, cornerPoints, cv::Size2i(11, 11), cv::Size2i(-1, -1), cv::TermCriteria(cv::TermCriteria::EPS | cv::TermCriteria::MAX_ITER, 30, 0.1));

		return true;
	}
	else {
		return false;
	}
}

IntrinsicParameters IntrinsicParameters::Calibrate(const cv::Size2i& imageSize, std::vector<std::vector<cv::Vec3f>>& objectPoints, std::vector<std::vector<cv::Vec2f>>& imagePoints)
{
	cv::Mat1f cameraMatrix;
	std::vector<float> distCoeffs;
	cv::Mat rvecs, tvecs;
	cv::calibrateCamera(objectPoints, imagePoints, imageSize, cameraMatrix, distCoeffs, rvecs, tvecs);

	IntrinsicParameters intr;

	intr.cx = cameraMatrix(0, 2);
	intr.cy = cameraMatrix(1, 2);
	intr.fx = cameraMatrix(0, 0);
	intr.fy = cameraMatrix(1, 1);

	intr.distortionRadial[0] = distCoeffs[0];
	intr.distortionRadial[1] = distCoeffs[1];
	intr.distortionRadial[2] = distCoeffs[4];

	intr.distortionPlanar[0] = distCoeffs[2];
	intr.distortionPlanar[1] = distCoeffs[3];

	return intr;
}

}