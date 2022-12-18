#ifndef __TRIANGULATOR_H__
#define __TRIANGULATOR_H__

#include <list>
#include <map>
#include <memory>

#include "Camera.h"
#include "EventSource.h"


namespace scanner
{

	struct RayInfo
	{
		cv::Point2f pixel;
		cv::Point2f undistorted;
		cv::Vec3f direction;
		float t;
	};

	class ITriangulatorCallback
	{
	public:
		virtual void onTriangulatorResult(const std::optional<cv::Point3f>& point3D, const std::map<const Camera*, std::optional<RayInfo>>& rayInfos) = 0;
	};


	class Triangulator
		: public EventSourceT<ITriangulatorCallback>
		, public ICameraCallback
	{
	public:
		typedef std::map<const Camera*, std::optional<cv::Point2f>> SampleHolder;

		static cv::Vec3f Normalize(cv::Vec3f v);
		static std::optional<cv::Point3f> Triangulate(const SampleHolder& samples, std::map<const Camera*, std::optional<RayInfo>>& rays);
		
	private:
		std::list<std::shared_ptr<Camera>> m_Cameras;

		std::mutex m_Mutex;
		SampleHolder m_Samples;

	public:
		Triangulator();
		~Triangulator();

		std::shared_ptr<Camera> addCamera(PS3EyeCamera&& device);
		void removeCamera(std::shared_ptr<Camera> camera);
		const std::list<std::shared_ptr<Camera>>& getCameras() const;
		void clearCameras();

	public: // ICameraCallback
		virtual void onFrame(const Camera* pCamera, const cv::Mat& colorBgr, const cv::Mat& undistortedBgr, const cv::Mat& thresholdedGray, const std::optional<cv::Point2f>& point, const std::chrono::high_resolution_clock::duration& imgProcTime) override;
	};

}

#endif // __TRIANGULATOR_H__