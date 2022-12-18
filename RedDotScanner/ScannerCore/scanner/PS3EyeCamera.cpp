#include "PS3EyeCamera.h"

#include <cassert>

#include <algorithm>
#include <stdexcept>

#include "Utils.h"


namespace scanner
{


// PS3EyeCamera ////////////////////////////////////////////////////////////////


PS3EyeCamera::PS3EyeCamera(ps3eye::PS3EYECam::PS3EYERef camera)
	: m_Camera(camera)
	, m_FrameWidth(0)
	, m_FrameHeight(0)
{
	bool bResult = m_Camera->init(640, 480, 30, ps3eye::PS3EYECam::EOutputFormat::Bayer);
	if (!bResult)
		throw std::runtime_error("Cannot initialize PS3EyeCamera!");

	m_FrameWidth = m_Camera->getWidth();
	m_FrameHeight = m_Camera->getHeight();
	assert(m_FrameWidth == 640 && m_FrameHeight == 480);

	m_Camera->setAutogain(false);
	m_Camera->setGain(0);

	m_Camera->start();
}

PS3EyeCamera::PS3EyeCamera(PS3EyeCamera&& other) noexcept
	: m_FrameWidth(0)
	, m_FrameHeight(0)
{
	*this = std::move(other);
}

PS3EyeCamera::~PS3EyeCamera()
{
	if (m_Camera)
		m_Camera->stop();
}

PS3EyeCamera& PS3EyeCamera::operator=(PS3EyeCamera&& other) noexcept
{
	if (this != &other) {
		m_Camera = std::move(other.m_Camera); // it's a shared_ptr so std::move is not really needed
		m_FrameWidth = other.m_FrameWidth;
		m_FrameHeight = other.m_FrameHeight;
	}

	return *this;
}

PS3EyeCamera::DeviceInfo PS3EyeCamera::getDeviceInfo() const
{
	char usbPortPath[256] = { 0 };
	m_Camera->getUSBPortPath(usbPortPath, sizeof(usbPortPath));

	DeviceInfo di;
	di.usb = usbPortPath;
	return di;
}

unsigned int PS3EyeCamera::getFrameWidth() const
{
	return m_FrameWidth;
}

unsigned int PS3EyeCamera::getFrameHeight() const
{
	return m_FrameHeight;
}

PixelFormat PS3EyeCamera::getPixelFormat() const
{
	return PixelFormat::BAYER;
}


uint8_t PS3EyeCamera::getExposure()
{
	return m_Camera->getExposure();
}

void PS3EyeCamera::setExposure(uint8_t exposure)
{
	m_Camera->setExposure(exposure);
}

bool PS3EyeCamera::getFrame(uint8_t* pPixels, const std::chrono::milliseconds& timeout)
{
	return m_Camera->getFrame(pPixels, timeout);
}


// PS3EyeCameraFactory /////////////////////////////////////////////////////////


std::vector<PS3EyeCamera::DeviceInfo> PS3EyeCameraFactory::getDevices()
{
	std::vector<PS3EyeCamera::DeviceInfo> devices;

	const auto& ps3eyes = ps3eye::PS3EYECam::getDevices(true);
	for (auto ps3eye : ps3eyes) {
		if (ps3eye->init(640, 480, 30, ps3eye::PS3EYECam::EOutputFormat::Bayer)) {
			char usbPortPath[256] = { 0 };

			if (ps3eye->getUSBPortPath(usbPortPath, sizeof(usbPortPath))) {
				PS3EyeCamera::DeviceInfo di;
				di.usb = usbPortPath;

				devices.push_back(di);
			}
		}
	}

	std::sort(devices.begin(), devices.end(),
		[] (const PS3EyeCamera::DeviceInfo& a, const PS3EyeCamera::DeviceInfo& b) {
			return a.usb < b.usb;
		}
	);

	return devices;
}

PS3EyeCamera PS3EyeCameraFactory::create(const std::string& usb)
{
	const auto& ps3eyes = ps3eye::PS3EYECam::getDevices(true);
	for (auto ps3eye : ps3eyes) {
		if (ps3eye->init(640, 480, 30, ps3eye::PS3EYECam::EOutputFormat::Bayer)) {
			char usbPortPath[256] = { 0 };

			if (ps3eye->getUSBPortPath(usbPortPath, sizeof(usbPortPath))) {
				if (0 == usb.compare(usbPortPath)) {
					return PS3EyeCamera(ps3eye);
				}
			}
		}
	}

	throw std::runtime_error(utils::S() << "Cannot find PS3EyeCamera (" << usb << ")!");
}

}