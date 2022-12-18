#ifndef __FRAMEGRABBERTHREAD_H__
#define __FRAMEGRABBERTHREAD_H__

#include <atomic>
#include <thread>

#include <opencv2/core.hpp>

#include "PS3EyeCamera.h"


namespace scanner
{

	class IFrameGrabberCallback
	{
	public:
		virtual void onFrame(const cv::Mat& frame) = 0;
	};


	class FrameGrabberThread
	{
		std::shared_ptr<PS3EyeCamera> m_Camera;
		IFrameGrabberCallback* m_pCallback;
		cv::Mat1b m_Frame;

		std::atomic<bool> m_bRunning;
		std::thread m_Thread;

		void threadFunc();

	public:
		FrameGrabberThread(const std::shared_ptr<PS3EyeCamera>& camera, IFrameGrabberCallback* pCallback);
		~FrameGrabberThread();

		void stop();
	};

}

#endif // __FRAMEGRABBERTHREAD_H__