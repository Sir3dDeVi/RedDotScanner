#ifndef __NXTLASERTURRET_H__
#define __NXTLASERTURRET_H__

#include <cstdint>

#include <chrono>
#include <mutex>
#include <thread>

#include "nxt/nxt.h"
#include "EventSource.h"

namespace scanner
{

	class INXTLaserTurretCallback
	{
	public:
		virtual void onNXTUpdate(uint8_t laserPower, int pan, uint8_t panSpeed, int tilt, uint8_t tiltSpeed) = 0;
	};

	class NXTLaserTurret : public EventSourceT<INXTLaserTurretCallback>
	{
		static const char* s_szFirmwareName;
		static const std::chrono::milliseconds s_FirmwareWarmupTime;

		static const uint8_t s_nControlMailbox;
		static const std::chrono::milliseconds s_ControlMailboxReadTimeout;

		static const char* s_szOutputPortNames[];
		static const size_t s_nOutputPortLaser;
		static const size_t s_nOutputPortPan;
		static const size_t s_nOutputPortTilt;

		static const uint8_t s_nUpdateMailbox;
		static const std::chrono::milliseconds s_UpdateThreadSleepTime;
		static const std::chrono::milliseconds s_KeepAliveTimeout;

	private:
		struct MotorState
		{
			int speed;
			int position;
		};

		MotorState getMotorState(const char* szOutputPort);

	private:
		std::mutex m_Mutex;
		nxt::NXT m_NXT;

		std::atomic<bool> m_bThreadRunning;
		std::thread m_Thread;
		std::chrono::high_resolution_clock::time_point m_lastKeepAlive;

		void threadFunc();

	public:
		explicit NXTLaserTurret(nxt::NXT&& nxt);
		~NXTLaserTurret();

		void setLaser(uint8_t power);
		uint8_t getLaser();

		void panRotate(uint8_t power);
		void panRotateTo(uint8_t power, int position);

		void titltRotate(uint8_t power);
		void tiltRotateTo(uint8_t power, int position);
	};

} // namespace scanner

#endif // __NXTLASERTURRET_H__