#include "Leveler.h"

#include <cassert>

#include <Utils.h>


namespace scanner
{

	const char* Leveler::s_szLevelerHost = "192.168.0.12";
	const size_t Leveler::s_IndexOrigo = 0;
	const size_t Leveler::s_IndexX = 2;
	const size_t Leveler::s_IndexY = 1;
	const float Leveler::s_DistanceX = 0.2f; // 20 cm
	const float Leveler::s_DistanceY = 0.15f; // 15 cm

	// TODO: move these functions to a separate file
	float Leveler::distance(const cv::Point3f& a, const cv::Point3f& b)
	{
		return sqrtf(
			(a.x - b.x) * (a.x - b.x) +
			(a.y - b.y) * (a.y - b.y) +
			(a.z - b.z) * (a.z - b.z)
		);
	}

	cv::Point3f Leveler::sub(const cv::Point3f& a, const cv::Point3f& b)
	{
		return cv::Point3f(
			a.x - b.x,
			a.y - b.y,
			a.z - b.z
		);
	}

	float Leveler::length(const cv::Point3f& v)
	{
		return sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
	}

	cv::Point3f Leveler::normalize(const cv::Point3f& v)
	{
		const float l = length(v);

		return cv::Point3f(
			v.x / l,
			v.y / l,
			v.z / l
		);
	}

	cv::Point3f Leveler::cross(const cv::Point3f& a, const cv::Point3f& b)
	{
		return cv::Point3f(
			a.y*b.z - a.z*b.y,
			a.z*b.x - a.x*b.z,
			a.x*b.y - a.y*b.x
		);
	}


	Leveler::Leveler(scanner::Triangulator& triangulator)
		: m_Triangulator(triangulator)
		, m_State(State::None)
		, m_CurrentLED(-1)
	{
	}

	void Leveler::start()
	{
		setLed(0, 0, 0, 0);
		setLed(1, 0, 0, 0);
		setLed(2, 0, 0, 0);

		// No need for mutexes, because we are not yet added to the Triangulator
		m_State = State::WaitingForLEDsTurnOff;
		m_CurrentLED = -1;

		m_Triangulator.addEventHandler(this);
	}

	void Leveler::onTriangulatorResult(const std::optional<cv::Point3f>& point3D, const std::map<const scanner::Camera*, std::optional<scanner::RayInfo>>& rayInfos)
	{
		switch (m_State) {
			case State::None:
				break;
			case State::WaitingForLEDsTurnOff: {
				bool bAnyOn = false;

				for (const auto& ri : rayInfos)
					bAnyOn |= ri.second.has_value();

				if (!bAnyOn)
				{
					++m_CurrentLED;
					m_State = State::WaitingForLEDTurnOn;

					setLed(m_CurrentLED, 255, 255, 255);
				}

				} break;
			case State::WaitingForLEDTurnOn:
				if (point3D.has_value()) {
					m_Points[m_CurrentLED] = *point3D;

					setLed(m_CurrentLED, 0, 0, 0);

					if (m_CurrentLED < 2) {
						m_State = State::WaitingForLEDsTurnOff;
					}
					else {
						m_State = State::None;

						finishLeveling();
					}
				}
				break;
		}
	}

	void Leveler::setLed(int led, uint8_t r, uint8_t g, uint8_t b)
	{
		std::string uri = utils::S() << "/led"
			<< "?led=" << led
			<< "&r=" << static_cast<int>(r)
			<< "&g=" << static_cast<int>(g)
			<< "&b=" << static_cast<int>(b);

		int status = utils::httpGet(s_szLevelerHost, uri.c_str());
		assert(status == 200);
	}

	void Leveler::finishLeveling()
	{
		m_Triangulator.removeEventHandler(this);

		const float scaleX = s_DistanceX / distance(m_Points[s_IndexOrigo], m_Points[s_IndexX]);
		const float scaleY = s_DistanceY / distance(m_Points[s_IndexOrigo], m_Points[s_IndexY]);
		const float scaleZ = (scaleX + scaleY) / 2.0f;

		const cv::Point3f& origo = m_Points[s_IndexOrigo];

		const cv::Point3f x = normalize(sub(m_Points[s_IndexX], m_Points[s_IndexOrigo]));
		const cv::Point3f yTmp = normalize(sub(m_Points[s_IndexY], m_Points[s_IndexOrigo]));
		const cv::Point3f z = normalize(cross(x, yTmp));
		const cv::Point3f y = normalize(cross(z, x)); // to be sure that x and y are perpendicular

		const cv::Matx44f rotate(
			x.x, x.y, x.z, 0.0f,
			y.x, y.y, y.z, 0.0f,
			z.x, z.y, z.z, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		);

		const cv::Matx44f scale(
			scaleX, 0.0f, 0.0f, 0.0f,
			0.0f, scaleY, 0.0f, 0.0f,
			0.0f, 0.0f, scaleZ, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		);

		const cv::Matx44f translate(
			1.0f, 0.0f, 0.0f, -origo.x,
			0.0f, 1.0f, 0.0f, -origo.y,
			0.0f, 0.0f, 1.0f, -origo.z,
			0.0f, 0.0f, 0.0f, 1.0f
		);

		const cv::Matx44f worldToLeveler = scale * rotate * translate;

		for (auto camera : m_Triangulator.getCameras()) {
			scanner::ExtrinsicParameters extr = camera->getExtrinsicParams();

			cv::Matx44f r(
				extr.cameraToWorld(0, 0), extr.cameraToWorld(0, 1), extr.cameraToWorld(0, 2), 0.0f,
				extr.cameraToWorld(1, 0), extr.cameraToWorld(1, 1), extr.cameraToWorld(1, 2), 0.0f,
				extr.cameraToWorld(2, 0), extr.cameraToWorld(2, 1), extr.cameraToWorld(2, 2), 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
			);
			r = worldToLeveler * r;
			extr.cameraToWorld = cv::Matx33f(
				r(0, 0), r(0, 1), r(0, 2),
				r(1, 0), r(1, 1), r(1, 2),
				r(2, 0), r(2, 1), r(2, 2)
			);

			cv::Vec4f p(extr.position.x, extr.position.y, extr.position.z, 1.0f);
			p = worldToLeveler * p;
			extr.position = cv::Point3f(p(0), p(1), p(2));

			camera->setExtrinsicParams(extr);
		}

		invoke(&ILevelerCallback::onLevelingFinished);
	}

} // namespace scanner