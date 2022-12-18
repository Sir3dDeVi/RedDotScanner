#ifndef __EXTRINSICPARAMETERS_H__
#define __EXTRINSICPARAMETERS_H__

#include <vector>

#include <opencv2/core.hpp>

#include "IntrinsicParameters.h"


namespace scanner
{

	struct ExtrinsicParameters
	{
		cv::Point3f position;
		cv::Matx33f cameraToWorld;

		ExtrinsicParameters();

		cv::Matx33f getWorldToCamera() const;
		cv::Matx13f getRodrigues() const;
		cv::Vec3f getT() const;

		static ExtrinsicParameters Calibrate(const IntrinsicParameters& intrinsicParams, std::vector<cv::Point3f>& objectPoints, std::vector<cv::Point2f>& imagePoints);
	};

}

#endif // __EXTRINSICPARAMETERS_H__