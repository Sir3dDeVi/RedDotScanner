#include "FrameGrabberThread.h"

#include <cassert>

#include <functional>

#include "Utils.h"


namespace scanner
{

FrameGrabberThread::FrameGrabberThread(const std::shared_ptr<PS3EyeCamera>& camera, IFrameGrabberCallback* pCallback)
	: m_Camera(camera)
	, m_pCallback(pCallback)
	, m_bRunning(false)
{
	m_Frame.create(cv::Size(m_Camera->getFrameWidth(), m_Camera->getFrameHeight()));

	m_bRunning = true;
	m_Thread = std::thread(std::bind(&FrameGrabberThread::threadFunc, this));
}

FrameGrabberThread::~FrameGrabberThread()
{
	stop();
}

void FrameGrabberThread::stop()
{
	if (m_bRunning) {
		m_bRunning = false;

		if (m_Thread.joinable())
			m_Thread.join();
	}
}

void FrameGrabberThread::threadFunc()
{
	utils::setThreadName(utils::S() << "FrameGrabberThread " << m_Camera.get());

	while (m_bRunning) {
		if (m_Camera->getFrame(m_Frame.ptr(), std::chrono::milliseconds(1)))
			m_pCallback->onFrame(m_Frame);
	}
}

}