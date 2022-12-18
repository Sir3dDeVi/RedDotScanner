#ifndef __INTRINSICPARAMETERS_H__
#define __INTRINSICPARAMETERS_H__

#include <vector>

#include <opencv2/core.hpp>


namespace scanner
{

	struct IntrinsicParameters
	{
		float cx, cy;
		float fx, fy;

		float distortionRadial[3];
		float distortionPlanar[2];

		IntrinsicParameters();

		cv::Mat1f getCameraMatrix() const;
		std::vector<float> getDistortionCoefficients() const;

		cv::Point2f undistortPixel(const cv::Point2f& distortedPixel) const;

		static bool FindChessboard(const cv::Mat3b& image, const cv::Size2i& chessboard, std::vector<cv::Vec2f>& cornerPoints);
		static IntrinsicParameters Calibrate(const cv::Size2i& imageSize, std::vector<std::vector<cv::Vec3f>>& objectPoints, std::vector<std::vector<cv::Vec2f>>& imagePoints);
	};

}

#endif // __INTRINSICPARAMETERS_H__