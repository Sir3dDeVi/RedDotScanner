#include "ExtrinsicParameters.h"

#include <opencv2/calib3d.hpp>


namespace scanner
{

ExtrinsicParameters::ExtrinsicParameters()
	: position(0.0f, 0.0f, 0.0f)
	, cameraToWorld(cv::Matx33f::eye())
{
}

cv::Matx33f ExtrinsicParameters::getWorldToCamera() const
{
	return cameraToWorld.t();
}

cv::Matx13f ExtrinsicParameters::getRodrigues() const
{
	cv::Matx13f rodrigues;
	cv::Rodrigues(getWorldToCamera(), rodrigues);

	return rodrigues;
}

cv::Vec3f ExtrinsicParameters::getT() const
{
	return -getWorldToCamera() * cv::Vec3f(position);
}

ExtrinsicParameters ExtrinsicParameters::Calibrate(const IntrinsicParameters& intrinsicParams, std::vector<cv::Point3f>& objectPoints, std::vector<cv::Point2f>& imagePoints)
{
	cv::Matx13f cvRVec;
	cv::Matx13f cvTVec;

	cv::solvePnP(
		objectPoints,
		imagePoints,
		intrinsicParams.getCameraMatrix(),
		intrinsicParams.getDistortionCoefficients(),
		cvRVec,
		cvTVec
	);

	ExtrinsicParameters extr;

	cv::Matx33f rotation;
	cv::Rodrigues(cvRVec, rotation);
	rotation = rotation.t();

	const cv::Vec3f t(cvTVec(0), cvTVec(1), cvTVec(2));
	const cv::Vec3f camPos = -rotation * t;
	extr.position = cv::Point3f(camPos(0), camPos(1), camPos(2));
	extr.cameraToWorld = rotation;

	return extr;
}

}