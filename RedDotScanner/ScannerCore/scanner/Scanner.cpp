#include "Scanner.h"

#include <fstream>
#include <iomanip>

#include <cvsba.h>
#ifdef SBA_DEBUG
#include <Windows.h>
#endif


namespace scanner
{

const std::chrono::milliseconds Scanner::s_MoveWarmupTime(1000);


Scanner::Scanner(Triangulator& triangulator, NXTLaserTurret& laserTurret)
	: m_Triangulator(triangulator)
	, m_LaserTurret(laserTurret)
	, m_bScanning(false)
	, m_LaserPower(0)
	, m_nSamplesU(0)
	, m_nSamplesV(0)
	, m_ScanningState(ScanningState::Moving)
	, m_ScanU(0)
	, m_ScanV(0)
{
	m_Triangulator.addEventHandler(this);
	m_LaserTurret.addEventHandler(this);
}

Scanner::~Scanner()
{
	m_Triangulator.removeEventHandler(this);
}

void Scanner::save(const std::string& filename)
{
	cv::FileStorage fs(filename, cv::FileStorage::WRITE, "utf-8");

	fs << "cameras" << "[";

	for (const auto& camera : m_Triangulator.getCameras()) {
		fs << "{";

		fs << "usb" << camera->getDevice()->getDeviceInfo().usb;

		fs << "resolution" << "{";
		fs << "width" << static_cast<int>(camera->getDevice()->getFrameWidth());
		fs << "height" << static_cast<int>(camera->getDevice()->getFrameHeight());
		fs << "}";

		const IntrinsicParameters& intr = camera->getIntrinsicParams();
		fs << "intrinsic" << "{";
		fs << "cx" << intr.cx;
		fs << "cy" << intr.cy;
		fs << "fx" << intr.fx;
		fs << "fy" << intr.fy;
		fs << "distortion-radial" << "[:" << intr.distortionRadial[0] << intr.distortionRadial[1] << intr.distortionRadial[2] << "]";
		fs << "distortion-planar" << "[:" << intr.distortionPlanar[0] << intr.distortionPlanar[1] << "]";
		fs << "}";

		const ExtrinsicParameters& extr = camera->getExtrinsicParams();
		fs << "extrinsic" << "{";
		fs << "position" << extr.position;
		fs << "camera-to-world" << extr.cameraToWorld;
		fs << "}";

		fs << "exposure" << camera->getDevice()->getExposure();
		fs << "threshold" << camera->getThreshold();

		fs << "}";
	}
	fs << "]";

	fs << "scan" << "{";
	fs << "samplesU" << m_nSamplesU;
	fs << "samplesV" << m_nSamplesV;
	fs << "points" << "[";
	for (const auto& scannedPoint : m_ScannedPoints) {
		fs << "{";

		fs << "point3D" << scannedPoint.point3D;

		fs << "point2Ds" << "[";
		for (const auto& camera : m_Triangulator.getCameras()) {
			auto it = scannedPoint.point2Ds.find(camera.get());
			assert(it != scannedPoint.point2Ds.end());

			const auto& rayInfo = it->second;
			if (rayInfo.has_value()) {
				fs << rayInfo->pixel;
			}
			else {
				fs << "[:" << "]";
			}
		}
		fs << "]";

		fs << "}";
	}
	fs << "]";
	fs << "}";

	fs.release();
}

void Scanner::load(const std::string& filename)
{
	// Clear state
	m_Triangulator.clearCameras();

	// Add cameras
	cv::FileStorage fs(filename, cv::FileStorage::READ, "utf-8");
	cv::FileNode fsCameras = fs["cameras"];

	std::vector<std::shared_ptr<Camera>> orderedCameras;

	PS3EyeCameraFactory ps3EyeFactory;

	for (const auto& fsCamera : fsCameras) {
		const std::string usb = fsCamera["usb"];

		auto camera = m_Triangulator.addCamera(std::move(ps3EyeFactory.create(usb)));
		orderedCameras.push_back(camera);


		IntrinsicParameters intr;
		cv::FileNode fsIntrinsic = fsCamera["intrinsic"];
		fsIntrinsic["cx"] >> intr.cx;
		fsIntrinsic["cy"] >> intr.cy;
		fsIntrinsic["fx"] >> intr.fx;
		fsIntrinsic["fy"] >> intr.fy;

		std::vector<float> distRadial, distPlanar;
		fsIntrinsic["distortion-radial"] >> distRadial;
		fsIntrinsic["distortion-planar"] >> distPlanar;

		assert(distRadial.size() == 3);
		assert(distPlanar.size() == 2);

		intr.distortionRadial[0] = distRadial[0];
		intr.distortionRadial[1] = distRadial[1];
		intr.distortionRadial[2] = distRadial[2];

		intr.distortionPlanar[0] = distPlanar[0];
		intr.distortionPlanar[1] = distPlanar[1];

		camera->setIntrinsicParams(intr);


		ExtrinsicParameters extr;
		cv::FileNode fsExtrinsic = fsCamera["extrinsic"];

		fsExtrinsic["position"] >> extr.position;
		fsExtrinsic["camera-to-world"] >> extr.cameraToWorld;

		camera->setExtrinsicParams(extr);


		camera->getDevice()->setExposure(static_cast<unsigned char>(static_cast<int>(fsCamera["exposure"])));
		camera->setThreshold(static_cast<unsigned char>(static_cast<int>(fsCamera["threshold"])));
	}

	// Add scan
	m_ScannedPoints.clear();
	cv::FileNode fsScan = fs["scan"];
	fsScan["samplesU"] >> m_nSamplesU;
	fsScan["samplesV"] >> m_nSamplesV;

	cv::FileNode fsPoints = fsScan["points"];
	for (const auto& fsScannedPoint : fsPoints) {
		cv::FileNode fsPoint2Ds = fsScannedPoint["point2Ds"];
		assert(fsPoint2Ds.size() == orderedCameras.size());

		Triangulator::SampleHolder samples;

		for (size_t i = 0; i < fsPoint2Ds.size(); ++i) {
			cv::FileNode fsPoint = fsPoint2Ds[i];

			std::optional<cv::Point2f> pixels;
			if (fsPoint.size() >= 2)
				pixels = cv::Point2f(fsPoint[0], fsPoint[1]);

			samples[orderedCameras[i].get()] = pixels;
		}


		ScannedPoint scannedPoint;
		auto p3d = Triangulator::Triangulate(samples, scannedPoint.point2Ds);
		assert(p3d.has_value());
		scannedPoint.point3D = *p3d;

		m_ScannedPoints.push_back(scannedPoint);
	}
}

Triangulator& Scanner::getTriangulator()
{
	return m_Triangulator;
}

NXTLaserTurret& Scanner::getLaserTurret()
{
	return m_LaserTurret;
}

std::vector<ScannedPoint> Scanner::getScannedPoints()
{
	std::lock_guard guard(m_Mutex);

	return m_ScannedPoints;
}

int Scanner::getSamplesU() const
{
	return m_nSamplesU;
}

int Scanner::getSamplesV() const
{
	return m_nSamplesV;
}

void Scanner::clearScannedPoints()
{
	std::lock_guard guard(m_Mutex);

	m_ScannedPoints.clear();
}

void Scanner::saveToObj(const std::string& filename)
{
	std::ofstream f(filename.c_str(), std::ios::out);
	f.setf(std::ios::fixed);
	f.precision(5);

	auto faceIndex = [this](int u, int v) {
		return 1 + v * m_nSamplesU + u;
	};

	f << "s off" << std::endl;
	f << std::endl;

	// "Bottom layer" - the captured data
	f << "# Scanned surface" << std::endl;
	float maxZ = -1000.0f;
	for (int v = 0; v < m_nSamplesV; ++v) {
#ifdef SCANNER_ZIGZAG
		if (v % 2 == 0) {
			for (int u = 0; u < m_nSamplesU; ++u) {
				const auto& sp = m_ScannedPoints[v * m_nSamplesU + u];
				f << "v " << sp.point3D.x << " " << sp.point3D.y << " " << sp.point3D.z << std::endl;
				maxZ = std::max(maxZ, sp.point3D.z);
			}
		}
		else {
			for (int u = m_nSamplesU - 1; u >= 0; --u) {
				const auto& sp = m_ScannedPoints[v * m_nSamplesU + u];
				f << "v " << sp.point3D.x << " " << sp.point3D.y << " " << sp.point3D.z << std::endl;
				maxZ = std::max(maxZ, sp.point3D.z);
			}
		}
#else
		for (int u = 0; u < m_nSamplesU; ++u) {
			const auto& sp = m_ScannedPoints[v * m_nSamplesU + u];
			f << "v " << sp.point3D.x << " " << sp.point3D.y << " " << sp.point3D.z << std::endl;
			maxZ = std::max(maxZ, sp.point3D.z);
		}
#endif
	}
	f << std::endl;

	for (int v = 0; v < m_nSamplesV - 1; ++v) {
		for (int u = 0; u < m_nSamplesU - 1; ++u) {
			f << "f " << faceIndex(u, v) << " " << faceIndex(u + 1, v) << " " << faceIndex(u + 1, v + 1) << std::endl;
			f << "f " << faceIndex(u, v) << " " << faceIndex(u + 1, v + 1) << " " << faceIndex(u, v + 1) << std::endl;
		}
	}
	f << std::endl;
	f << std::endl;

	// Top layer
	f << "# Top layer" << std::endl;
	for (int v = 0; v < m_nSamplesV; ++v) {
#ifdef SCANNER_ZIGZAG
		if (v % 2 == 0) {
			for (int u = 0; u < m_nSamplesU; ++u) {
				const auto& sp = m_ScannedPoints[v * m_nSamplesU + u];
				f << "v " << sp.point3D.x << " " << sp.point3D.y << " " << maxZ + 0.01f << std::endl;
			}
		}
		else {
			for (int u = m_nSamplesU - 1; u >= 0; --u) {
				const auto& sp = m_ScannedPoints[v * m_nSamplesU + u];
				f << "v " << sp.point3D.x << " " << sp.point3D.y << " " << maxZ + 0.01f << std::endl;
			}
		}
#else
		for (int u = 0; u < m_nSamplesU; ++u) {
			const auto& sp = m_ScannedPoints[v * m_nSamplesU + u];
			f << "v " << sp.point3D.x << " " << sp.point3D.y << " " << maxZ + 0.01f << std::endl;
		}
#endif
	}
	f << std::endl;
	
	const int nVertexes = m_nSamplesU * m_nSamplesV;
	for (int v = 0; v < m_nSamplesV - 1; ++v) {
		for (int u = 0; u < m_nSamplesU - 1; ++u) {
			f << "f " << nVertexes + faceIndex(u, v) << " " << nVertexes + faceIndex(u + 1, v) << " " << nVertexes + faceIndex(u + 1, v + 1) << std::endl;
			f << "f " << nVertexes + faceIndex(u, v) << " " << nVertexes + faceIndex(u + 1, v + 1) << " " << nVertexes + faceIndex(u, v + 1) << std::endl;
		}
	}
	f << std::endl;

	// Sides
	f << "# Sides" << std::endl;
	for (int u = 0; u < m_nSamplesU - 1; ++u) {
		f << "f " << faceIndex(u, 0) << " " << nVertexes + faceIndex(u, 0) << " " << nVertexes + faceIndex(u + 1, 0) << std::endl;
		f << "f " << faceIndex(u, 0) << " " << nVertexes + faceIndex(u + 1, 0) << " " << faceIndex(u + 1, 0) << std::endl;
	}
	f << std::endl;

	for (int u = 0; u < m_nSamplesU - 1; ++u) {
		f << "f " << faceIndex(u, m_nSamplesV - 1) << " " << nVertexes + faceIndex(u, m_nSamplesV - 1) << " " << nVertexes + faceIndex(u + 1, m_nSamplesV - 1) << std::endl;
		f << "f " << faceIndex(u, m_nSamplesV - 1) << " " << nVertexes + faceIndex(u + 1, m_nSamplesV - 1) << " " << faceIndex(u + 1, m_nSamplesV - 1) << std::endl;
	}
	f << std::endl;

	for (int v = 0; v < m_nSamplesV - 1; ++v) {
		f << "f " << faceIndex(0, v) << " " << nVertexes + faceIndex(0, v) << " " << nVertexes + faceIndex(0, v + 1) << std::endl;
		f << "f " << faceIndex(0, v) << " " << nVertexes + faceIndex(0, v + 1) << " " << faceIndex(0, v + 1) << std::endl;
	}
	f << std::endl;

	for (int v = 0; v < m_nSamplesV - 1; ++v) {
		f << "f " << faceIndex(m_nSamplesU - 1, v) << " " << nVertexes + faceIndex(m_nSamplesU - 1, v) << " " << nVertexes + faceIndex(m_nSamplesU - 1, v + 1) << std::endl;
		f << "f " << faceIndex(m_nSamplesU - 1, v) << " " << nVertexes + faceIndex(m_nSamplesU - 1, v + 1) << " " << faceIndex(m_nSamplesU - 1, v + 1) << std::endl;
	}
	f << std::endl;
}

void Scanner::gotoUV(int u, int v)
{
	// A----------B
	// |          |
	// |          |
	// |          |
	// D----------C

	float uu = static_cast<float>(u) / static_cast<float>(m_nSamplesU - 1);
	float vv = static_cast<float>(v) / static_cast<float>(m_nSamplesV - 1);

	PanTilt ab(
		static_cast<int>( (1.0f - uu) * m_ScanAreaA.pan + uu * m_ScanAreaB.pan ),
		static_cast<int>( (1.0f - uu) * m_ScanAreaA.tilt + uu * m_ScanAreaB.tilt )
	);

	PanTilt cd(
		static_cast<int>( (1.0f - uu) * m_ScanAreaD.pan + uu * m_ScanAreaC.pan ),
		static_cast<int>( (1.0f - uu) * m_ScanAreaD.tilt + uu * m_ScanAreaC.tilt )
	);

	m_CurrentTarget.pan = static_cast<int>( (1.0f - vv) * ab.pan + vv * cd.pan );
	m_CurrentTarget.tilt = static_cast<int>( (1.0f - vv) * ab.tilt + vv * cd.tilt );

	//utils::debugPrint(utils::S()
	//	<< std::fixed << std::setprecision(5) << uu << " " << std::fixed << std::setprecision(5) << vv << "  "
	//	<< m_CurrentTarget.pan << " " << m_CurrentTarget.tilt
	//);

	m_LaserTurret.panRotateTo(100, m_CurrentTarget.pan);
	m_LaserTurret.tiltRotateTo(100, m_CurrentTarget.tilt);

	m_MoveStartTime = std::chrono::high_resolution_clock::now();

#ifdef SCANNER_DEBUG
	utils::debugPrint(utils::S() << "gotoUV(" << u << ", " << v << ") [" << m_CurrentTarget.pan << ", " << m_CurrentTarget.tilt << "]");
#endif
}

void Scanner::startScanning(const PanTilt& scanAreaA, const PanTilt& scanAreaB, const PanTilt& scanAreaC, const PanTilt& scanAreaD, int nSamplesU, int nSamplesV)
{
	std::lock_guard guard(m_Mutex);

	m_LaserPower = m_LaserTurret.getLaser();

	m_ScanAreaA = scanAreaA;
	m_ScanAreaB = scanAreaB;
	m_ScanAreaC = scanAreaC;
	m_ScanAreaD = scanAreaD;

	m_nSamplesU = nSamplesU;
	m_nSamplesV = nSamplesV;

	m_ScanU = 0;
	m_ScanV = 0;

	m_bScanning = true;

	m_LaserTurret.setLaser(10);
	m_ScanningState = ScanningState::Moving;

	m_ScannedPoints.clear();

	gotoUV(0, 0);
}

void Scanner::stopScanning()
{
	std::lock_guard guard(m_Mutex);

	m_bScanning = false;

	m_LaserTurret.panRotate(0);
	m_LaserTurret.titltRotate(0);
	m_LaserTurret.setLaser(m_LaserPower);
}

bool Scanner::isScanning()
{
	std::lock_guard guard(m_Mutex);

	return m_bScanning;
}

void Scanner::runBundleAdjustment(double& initReprojError, double& calibRerpojError)
{
	// Prepare data for SBA
	std::vector<std::shared_ptr<Camera>> cameras;
	for (auto c : m_Triangulator.getCameras())
		cameras.push_back(c);

	// Prepare data for SBA - Points
	std::vector<cv::Point3f> points;
	std::vector<std::vector<cv::Point2f>> imagePoints;
	std::vector<std::vector<int>> visibility;

	for (size_t i = 0; i < cameras.size(); ++i) {
		imagePoints.push_back(std::vector<cv::Point2f>());
		visibility.push_back(std::vector<int>());
	}

	for (const auto& sp : m_ScannedPoints) {
		points.push_back(sp.point3D);

		for (size_t i = 0; i < cameras.size(); ++i) {
			const Camera* pCamera = cameras[i].get();
			auto it = sp.point2Ds.find(pCamera);
			assert(it != sp.point2Ds.end());
			const std::optional<RayInfo>& rayInfo = it->second;

			if (rayInfo.has_value()) {
				imagePoints[i].push_back(rayInfo->pixel);
				visibility[i].push_back(1);
			}
			else {
				imagePoints[i].push_back(cv::Point2f(-1.0f, -1.0f));
				visibility[i].push_back(0);
			}
		}
	}

	// Prepare data for SBA - Cameras
	std::vector<cv::Mat> cameraIntrinsicMatrixes;
	std::vector<cv::Mat> cameraDistortionCoefficients;
	std::vector<cv::Mat> cameraRotationMatrixes;
	std::vector<cv::Mat> cameraTranslations;
	for (size_t i = 0; i < cameras.size(); ++i) {
		auto camera = cameras[i];

		cameraIntrinsicMatrixes.push_back(camera->getIntrinsicParams().getCameraMatrix());
		cameraDistortionCoefficients.push_back(cv::Mat(camera->getIntrinsicParams().getDistortionCoefficients(), true));

		cameraRotationMatrixes.push_back(cv::Mat(camera->getExtrinsicParams().getWorldToCamera(), true));
		cameraTranslations.push_back(cv::Mat(camera->getExtrinsicParams().getT(), true));
	}

	// Run SBA
	cvsba::Sba sba;
	sba.setParams(cvsba::Sba::Params(
		cvsba::Sba::MOTIONSTRUCTURE,
		150,
		1e-10,
		5,
		5,
#ifdef SBA_DEBUG
		true
#else
		false
#endif
	));

#ifdef SBA_DEBUG
	AllocConsole();
	FILE* fDummy;
	freopen_s(&fDummy, "CONOUT$", "w", stdout);
	freopen_s(&fDummy, "CONOUT$", "w", stderr);
	freopen_s(&fDummy, "CONIN$", "r", stdin);
#endif

	sba.run(
		points,
		imagePoints,
		visibility,
		cameraIntrinsicMatrixes,
		cameraRotationMatrixes,
		cameraTranslations,
		cameraDistortionCoefficients
	);

#ifdef SBA_DEBUG
	fprintf(stdout, "done\n");
	fprintf(stderr, "done\n");
#endif

	initReprojError = sba.getInitialReprjError();
	calibRerpojError = sba.getFinalReprjError();

	// Copy back results from SBA
	for (size_t i = 0; i < cameras.size(); ++i) {
		const cv::Mat_<double> cvIntr = cameraIntrinsicMatrixes[i];
		const cv::Mat_<double> cvDist = cameraDistortionCoefficients[i];
		const cv::Mat_<double> cvRot = cameraRotationMatrixes[i];
		const cv::Mat_<double> cvT = cameraTranslations[i];

		auto camera = cameras[i];

		// TODO: Refactor this to IntrinsicParameters
		IntrinsicParameters intr;
		intr.cx = static_cast<float>(cvIntr(0, 2));
		intr.cy = static_cast<float>(cvIntr(1, 2));
		intr.fx = static_cast<float>(cvIntr(0, 0));
		intr.fy = static_cast<float>(cvIntr(1, 1));

		intr.distortionRadial[0] = static_cast<float>(cvDist(0));
		intr.distortionRadial[1] = static_cast<float>(cvDist(1));
		intr.distortionRadial[2] = static_cast<float>(cvDist(4));

		intr.distortionPlanar[0] = static_cast<float>(cvDist(2));
		intr.distortionPlanar[1] = static_cast<float>(cvDist(3));

		camera->setIntrinsicParams(intr);
		
		// TODO: Refactor this to ExtrinsicParameters
		ExtrinsicParameters extr;
		extr.cameraToWorld = cv::Mat(cvRot.t());
		extr.position = -extr.cameraToWorld * cv::Vec3f(
			static_cast<float>(cvT(0)), static_cast<float>(cvT(1)), static_cast<float>(cvT(2)));
		camera->setExtrinsicParams(extr);
	}

	// Updated the 3D position of the scanned points
	for (auto& sp : m_ScannedPoints) {
		Triangulator::SampleHolder sh;

		for (const auto& s : sp.point2Ds)
			sh[s.first] = s.second->pixel;

		auto p = Triangulator::Triangulate(sh, sp.point2Ds);
		assert(p.has_value());
		sp.point3D = p.value();
	}
}

void Scanner::onNXTUpdate(uint8_t laserPower, int pan, uint8_t panSpeed, int tilt, uint8_t tiltSpeed)
{
	std::lock_guard guard(m_Mutex);

	if (m_bScanning) {
#ifdef SCANNER_DEBUG
		utils::debugPrint(utils::S() << "nxt: [" << pan << ", " << tilt << "]");
#endif

		auto now = std::chrono::high_resolution_clock::now();

		// Detect when stopped
		if (m_ScanningState == ScanningState::Moving) {

			// Give the LaserTurret some warmup time when it's starting to move
			const bool bLaserTurretWarmedUp = (now - m_MoveStartTime) >= s_MoveWarmupTime;

			if (bLaserTurretWarmedUp && panSpeed == 0 && tiltSpeed == 0) {
#ifdef SCANNER_DEBUG
				utils::debugPrint(utils::S()
					<< m_CurrentTarget.pan << " " << m_CurrentTarget.tilt << "  "
					<< pan << " " << tilt
				);
#endif

				m_ScanningState = ScanningState::Scanning;

				m_LaserTurret.setLaser(m_LaserPower);
			}
		}
	}
}

void Scanner::onTriangulatorResult(const std::optional<cv::Point3f>& point, const std::map<const Camera*, std::optional<RayInfo>>& rays)
{
	std::lock_guard guard(m_Mutex);

	if (m_bScanning) {
		if (m_ScanningState == ScanningState::Scanning && point.has_value()) {
			ScannedPoint scannedPoint;
			scannedPoint.point3D = *point;
			scannedPoint.point2Ds = rays;

			m_ScannedPoints.push_back(scannedPoint);

			invoke(&IScannerCallback::onScanningProgress, m_ScanU, m_ScanV, m_ScannedPoints);

			m_ScanningState = ScanningState::WaitingForLaserToTurnOff;
			m_LaserTurret.setLaser(10);
		}
		else if (m_ScanningState == ScanningState::WaitingForLaserToTurnOff && !point.has_value()) {
			// Go to next point or finish scanning
			bool finished = false;

#ifdef SCANNER_ZIGZAG
			if (m_ScanV % 2 == 0) {
				++m_ScanU;

				if (m_ScanU == m_nSamplesU) {
					m_ScanU = m_nSamplesU - 1;
					++m_ScanV;

					if (m_ScanV == m_nSamplesV)
						finished = true;
				}
			}
			else {
				--m_ScanU;

				if (m_ScanU == -1) {
					m_ScanU = 0;
					++m_ScanV;

					if (m_ScanV == m_nSamplesV)
						finished = true;
				}
			}
#else
			++m_ScanU;

			if (m_ScanU == m_nSamplesU) {
				m_ScanU = 0;
				++m_ScanV;

				if (m_ScanV == m_nSamplesV)
					finished = true;
			}
#endif

			if (!finished) {
				m_ScanningState = ScanningState::Moving;
				gotoUV(m_ScanU, m_ScanV);
			}
			else {
				m_bScanning = false;

				m_LaserTurret.setLaser(m_LaserPower);

				invoke(&IScannerCallback::onScanningFinished);
			}
		}
	}
}

}