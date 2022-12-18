#ifndef __SCANNER_H__
#define __SCANNER_H__

#include <chrono>
#include <memory>
#include <mutex>

#include "nxt/NXT.h"
#include "EventSource.h"
#include "NXTLaserTurret.h"
#include "Triangulator.h"


namespace scanner
{

	struct ScannedPoint
	{
		std::map<const Camera*, std::optional<RayInfo>> point2Ds;
		cv::Point3f point3D;
	};

	class IScannerCallback
	{
	public:
		virtual void onScanningProgress(int u, int v, const std::vector<ScannedPoint>& scannedPoints) = 0;
		virtual void onScanningFinished() = 0;
	};

	struct PanTilt
	{
		int pan;
		int tilt;

		PanTilt()
			: pan(0)
			, tilt(0)
		{
		}

		PanTilt(int pan, int tilt)
			: pan(pan)
			, tilt(tilt)
		{
		}
	};

	class Scanner
		: public INXTLaserTurretCallback
		, public ITriangulatorCallback
		, public EventSourceT<IScannerCallback>
	{
		static const std::chrono::milliseconds s_MoveWarmupTime;

		enum class ScanningState { Moving, Scanning, WaitingForLaserToTurnOff };

		Triangulator& m_Triangulator;
		NXTLaserTurret& m_LaserTurret;

		std::mutex m_Mutex;
		bool m_bScanning;
		uint8_t m_LaserPower;
		PanTilt m_ScanAreaA, m_ScanAreaB, m_ScanAreaC, m_ScanAreaD;
		int m_nSamplesU, m_nSamplesV;

		ScanningState m_ScanningState;
		int m_ScanU, m_ScanV;
		PanTilt m_CurrentTarget;
		std::chrono::high_resolution_clock::time_point m_MoveStartTime;

		std::vector<ScannedPoint> m_ScannedPoints;

		void gotoUV(int u, int v);
	public:
		Scanner(Triangulator& triangulator, NXTLaserTurret& laserTurret);
		~Scanner();

		void save(const std::string& filename);
		void load(const std::string& filename);

		Triangulator& getTriangulator();
		NXTLaserTurret& getLaserTurret();

		std::vector<ScannedPoint> getScannedPoints(); // not a const ref, because it might change from a background thread... so copy it
		int getSamplesU() const;
		int getSamplesV() const;
		void clearScannedPoints();
		void saveToObj(const std::string& filename);

		void startScanning(const PanTilt& scanAreaA, const PanTilt& scanAreaB, const PanTilt& scanAreaC, const PanTilt& scanAreaD, int nSamplesU, int nSamplesV);
		void stopScanning();
		bool isScanning();

		void runBundleAdjustment(double& initReprojError, double& calibRerpojError);

	public:
		// INXTLaserTurretCallback
		virtual void onNXTUpdate(uint8_t laserPower, int pan, uint8_t panSpeed, int tilt, uint8_t tiltSpeed) override;

		// ITriangulatorCallback
		virtual void onTriangulatorResult(const std::optional<cv::Point3f>& point3D, const std::map<const Camera*, std::optional<RayInfo>>& rayInfos) override;
	};

}

#endif // __SCANNER_H__