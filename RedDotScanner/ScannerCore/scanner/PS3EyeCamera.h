#ifndef __PS3EYECAMERA_H__
#define __PS3EYECAMERA_H__

#include <cstdint>

#include <chrono>
#include <mutex>
#include <string>

#include <ps3eye.h>

#include "Types.h"


namespace scanner
{

	class PS3EyeCameraFactory;


	class PS3EyeCamera
	{
	public:
		struct DeviceInfo
		{
			std::string usb;
		};

	private:
		ps3eye::PS3EYECam::PS3EYERef m_Camera;
		unsigned int m_FrameWidth;
		unsigned int m_FrameHeight;

	private:
		friend class PS3EyeCameraFactory;

		explicit PS3EyeCamera(ps3eye::PS3EYECam::PS3EYERef camera);

	public: // resource management
		PS3EyeCamera(const PS3EyeCamera& other) = delete;
		PS3EyeCamera(PS3EyeCamera&& other) noexcept;

		~PS3EyeCamera();

		PS3EyeCamera& operator=(const PS3EyeCamera& other) = delete;
		PS3EyeCamera& operator=(PS3EyeCamera&& other) noexcept;

	public:
		DeviceInfo getDeviceInfo() const;
		unsigned int getFrameWidth() const;
		unsigned int getFrameHeight() const;
		PixelFormat getPixelFormat() const;

		uint8_t getExposure();
		void setExposure(uint8_t exposure);

		bool getFrame(uint8_t* pPixels, const std::chrono::milliseconds& timeout);
	};

	
	class PS3EyeCameraFactory
	{
	public:
		std::vector<PS3EyeCamera::DeviceInfo> getDevices();
		PS3EyeCamera create(const std::string& usb);
	};

} // namespace scanner

#endif // __PS3EYECAMERA_H__
