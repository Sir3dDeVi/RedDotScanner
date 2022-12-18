#include "Triangulator.h"


namespace scanner
{

cv::Vec3f Triangulator::Normalize(cv::Vec3f v)
{
	float l = sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);

	v[0] /= l;
	v[1] /= l;
	v[2] /= l;

	return v;
}

std::optional<cv::Point3f> Triangulator::Triangulate(const SampleHolder& samples, std::map<const Camera*, std::optional<RayInfo>>& rays)
{
	// Prepare rays
	size_t nSamples = 0;

	for (const auto& s : samples) {
		const Camera* pCamera = s.first;
		const IntrinsicParameters& intr = pCamera->getIntrinsicParams();
		const ExtrinsicParameters& extr = pCamera->getExtrinsicParams();

		const std::optional<cv::Point2f>& pointPixel = s.second;

		std::optional<RayInfo> rayInfo;

		if (pointPixel.has_value()) {
			++nSamples;

			const cv::Point2f undistortedPixel = intr.undistortPixel(*pointPixel);

			// Inverse intrinsic to get a vector which points towards the point in the camera
			// coordinate system (camera in the origo and looking towards +Z).
			// Or, this is the 3D position of the point if it is projected to Z=1 in the camera
			// coordinate system.
			const cv::Vec3f pointDirInCamera(
				(undistortedPixel.x - intr.cx) / intr.fx,
				(undistortedPixel.y - intr.cy) / intr.fy,
				1.0f
			);

			// Direction towards the sample from the position of the camera in the world
			// coordinate system.
			const cv::Vec3f pointDirInWorld = Normalize(extr.cameraToWorld * pointDirInCamera);

			// Store results
			RayInfo ri;
			ri.pixel = *pointPixel;
			ri.undistorted = undistortedPixel;
			ri.direction = pointDirInWorld;
			ri.t = 10.0f; // we don't know (yet), but somewhere along this line (this is used during the 3D visualization)

			rayInfo = ri;
		}

		rays[pCamera] = rayInfo;
	}

	if (nSamples < 2)
		return {};


	// Triangulation
	cv::Mat1f A(cv::Size(3 + nSamples, 3 * nSamples), 0.0f);
	cv::Mat1f x(cv::Size(1, 3 + nSamples), 0.0f);
	cv::Mat1f B(cv::Size(1, 3 * nSamples), 0.0f);

	size_t i = 0;
	for (const auto& s : samples) {
		const Camera* pCamera = s.first;
		const ExtrinsicParameters& extr = pCamera->getExtrinsicParams();

		const std::optional<RayInfo>& rayInfo = rays[pCamera];

		if (rayInfo.has_value()) {
			A(3 * i, 0) = 1.0f;
			A(3 * i + 1, 1) = 1.0f;
			A(3 * i + 2, 2) = 1.0f;

			A(3 * i, 3 + i) = -rayInfo->direction(0);
			A(3 * i + 1, 3 + i) = -rayInfo->direction(1);
			A(3 * i + 2, 3 + i) = -rayInfo->direction(2);

			B(3 * i) = extr.position.x;
			B(3 * i + 1) = extr.position.y;
			B(3 * i + 2) = extr.position.z;

			++i;
		}
	}

	cv::solve(A, B, x, cv::DECOMP_SVD);

	// Check validity
	for (size_t i = 0; i < nSamples; ++i) {
		const float t = x(3 + i);
		if (t <= 0.0f)
			return {};
	}

	// Populate results
	i = 0;
	for (const auto& s : samples) {
		const Camera* pCamera = s.first;
		std::optional<RayInfo>& rayInfo = rays[pCamera];

		if (rayInfo.has_value()) {
			rayInfo->t = x(3 + i);

			++i;
		}
	}

	return cv::Point3f(x(0), x(1), x(2));;
}


Triangulator::Triangulator()
{

}

Triangulator::~Triangulator()
{
	auto cameras = m_Cameras;
	for (auto cam : cameras)
		removeCamera(cam);
}

std::shared_ptr<Camera> Triangulator::addCamera(PS3EyeCamera&& device)
{
	std::lock_guard guard(m_Mutex);

	auto camera = std::make_shared<Camera>(std::move(device));
	camera->addEventHandler(this);

	m_Cameras.push_back(camera);
	m_Samples.insert(std::make_pair(camera.get(), std::optional<cv::Point2f>()));

	return camera;
}

void Triangulator::removeCamera(std::shared_ptr<Camera> camera)
{
	std::lock_guard guard(m_Mutex);

	camera->removeEventHandler(this);

	for (auto it = m_Cameras.begin(); it != m_Cameras.end(); ++it) {
		if (*it == camera) {
			m_Cameras.erase(it);

			auto it = m_Samples.find(camera.get());
			assert(it != m_Samples.end());
			m_Samples.erase(it);

			return;
		}
	}

	assert(false);
}

const std::list<std::shared_ptr<Camera>>& Triangulator::getCameras() const
{
	return m_Cameras;
}

void Triangulator::clearCameras()
{
	auto cameras = m_Cameras;
	for (auto cam : cameras)
		removeCamera(cam);
}

void Triangulator::onFrame(const Camera* pCamera, const cv::Mat& colorBgr, const cv::Mat& undistortedBgr, const cv::Mat& thresholdedGray, const std::optional<cv::Point2f>& point, const std::chrono::high_resolution_clock::duration& imgProcTime)
{
	std::lock_guard guard(m_Mutex);

	// Update sample
	auto it = m_Samples.find(pCamera);
	assert(it != m_Samples.end());

	it->second = point;

	// Calculate 3D position
	std::map<const Camera*, std::optional<RayInfo>> rays;
	std::optional<cv::Point3f> triangulatedPoint = Triangulate(m_Samples, rays);

	invoke(&ITriangulatorCallback::onTriangulatorResult, triangulatedPoint, rays);
}

}