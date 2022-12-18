#ifndef __LEVELER_H__
#define __LEVELER_H__

#include <functional>

#include "Triangulator.h"
#include "EventSource.h"


namespace scanner
{
	class ILevelerCallback
	{
	public:
		virtual void onLevelingFinished() = 0;
	};


	class Leveler
		: public ITriangulatorCallback
		, public EventSourceT<ILevelerCallback>
	{
	private:
		static const char* s_szLevelerHost;
		static const size_t s_IndexOrigo;
		static const size_t s_IndexX;
		static const size_t s_IndexY;
		static const float s_DistanceX;
		static const float s_DistanceY;

		static float distance(const cv::Point3f& a, const cv::Point3f& b);
		static cv::Point3f sub(const cv::Point3f& a, const cv::Point3f& b);
		static float length(const cv::Point3f& a);
		static cv::Point3f normalize(const cv::Point3f& v);
		static cv::Point3f cross(const cv::Point3f& a, const cv::Point3f& b);

	private:
		enum class State { None, WaitingForLEDsTurnOff, WaitingForLEDTurnOn };

		scanner::Triangulator& m_Triangulator;
		State m_State;
		int m_CurrentLED;
		cv::Point3f m_Points[3];

		void setLed(int led, uint8_t r, uint8_t g, uint8_t b);
		void finishLeveling();
	public:
		Leveler(Triangulator& triangulator);

		void start();

	public: // ITriangulatorCallback
		virtual void onTriangulatorResult(const std::optional<cv::Point3f>& point3D, const std::map<const Camera*, std::optional<RayInfo>>& rayInfos) override;
	};
}

#endif // __LEVELER_H__