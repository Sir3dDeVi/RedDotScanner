#include "pch.h"
#include "LaserTurret.h"
#include "LaserTurret.g.cpp"

#include "ViewModel/Scanner.h"


namespace winrt::RedDotScanner::implementation
{

    LaserTurret::LaserTurret(Microsoft::UI::Dispatching::DispatcherQueue const& rtDispatcherQueue)
        : LaserTurret(rtDispatcherQueue, nullptr)
    {
    }

    LaserTurret::LaserTurret(Microsoft::UI::Dispatching::DispatcherQueue const& rtDispatcherQueue, Scanner* prtiScanner)
        : m_rtDispatcherQueue(rtDispatcherQueue)
        , m_prtiScanner(prtiScanner)
        , m_rtNXTDevices(winrt::single_threaded_observable_vector<RedDotScanner::NXTDeviceInfo>())
        , m_Laser(0)
        , m_Pan(0)
        , m_PanSpeed(0)
        , m_Tilt(0)
        , m_TiltSpeed(0)
    {
        RefreshNXTDevices();
        if (m_rtNXTDevices.Size() >= 2)
            CurrentNXTDevice(m_rtNXTDevices.GetAt(1));
        else
            CurrentNXTDevice(m_rtNXTDevices.GetAt(0));
    }

    void LaserTurret::Close()
    {
        m_NXTLaserTurret.reset();
    }


    void LaserTurret::RefreshNXTDevices()
    {
        m_rtNXTDevices.Clear();
        m_rtNXTDevices.Append(RedDotScanner::NXTDeviceInfo(L"(none)", L""));

        nxt::Factory nxtFactory;
        const auto devices = nxtFactory.getAvailableDevices();
        for (const auto& device : devices) {
            m_rtNXTDevices.Append(RedDotScanner::NXTDeviceInfo(
                to_hstring(device.name), to_hstring(device.resourceName)
            ));
        }

        RaisePropertyChanged(L"NXTDevices");
    }

    winrt::Windows::Foundation::Collections::IObservableVector<winrt::RedDotScanner::NXTDeviceInfo> LaserTurret::NXTDevices()
    {
        return m_rtNXTDevices;
    }

    winrt::RedDotScanner::NXTDeviceInfo LaserTurret::CurrentNXTDevice()
    {
        return m_rtCurrentNXTDeVice;
    }

    void LaserTurret::CurrentNXTDevice(winrt::RedDotScanner::NXTDeviceInfo const& value)
    {
        if (value != m_rtCurrentNXTDeVice)
        {
            m_rtCurrentNXTDeVice = value;

            m_NXTLaserTurret = nullptr;

            if (!m_rtCurrentNXTDeVice.ResourceName().empty())
            {
                nxt::Factory nxtFactory;
                try {
                    m_NXTLaserTurret = std::make_unique<scanner::NXTLaserTurret>(
                        nxtFactory.create(to_string(m_rtCurrentNXTDeVice.ResourceName()))
                    );
                    m_NXTLaserTurret->addEventHandler(this);
                }
                catch (std::exception& ex) {
                    OutputDebugStringA(ex.what());

                    m_NXTLaserTurret = nullptr;
                    m_rtCurrentNXTDeVice = m_rtNXTDevices.GetAt(0);
                }
            }

            if (m_prtiScanner)
                m_prtiScanner->setLaserTurret(m_NXTLaserTurret.get());

            m_Laser = 0;
            m_Pan = 0;
            m_Tilt = 0;
            m_PanSpeed = 0;
            m_TiltSpeed = 0;

            RaisePropertyChanged(L"");
        }
    }


    bool LaserTurret::IsAvailable() const
    {
        return m_NXTLaserTurret != nullptr;
    }

    uint8_t LaserTurret::LaserPower()
    {
        return m_Laser;
    }

    void LaserTurret::LaserPower(uint8_t value)
    {
        if (m_NXTLaserTurret && value != m_Laser)
        {
            m_NXTLaserTurret->setLaser(value);
            m_Laser = value;
        }

        RaisePropertyChanged(L"LaserPower");
    }

    int32_t LaserTurret::Pan()
    {
        return m_Pan;
    }

    uint8_t LaserTurret::PanSpeed()
    {
        return m_PanSpeed;
    }

    int32_t LaserTurret::Tilt()
    {
        return m_Tilt;
    }

    uint8_t LaserTurret::TiltSpeed()
    {
        return m_TiltSpeed;
    }

    void LaserTurret::TiltRotate(int16_t speed)
    {
        if (m_NXTLaserTurret)
        {
            m_NXTLaserTurret->titltRotate(static_cast<int8_t>(speed));
        }
    }

    void LaserTurret::PanRotate(int16_t speed)
    {
        if (m_NXTLaserTurret)
        {
            m_NXTLaserTurret->panRotate(static_cast<int8_t>(speed));
        }
    }

    void LaserTurret::RotateTo(int32_t pan, uint8_t panSpeed, int32_t tilt, uint8_t tiltSpeed)
    {
        if (m_NXTLaserTurret)
        {
            m_NXTLaserTurret->panRotateTo(panSpeed, pan);
            m_NXTLaserTurret->tiltRotateTo(tiltSpeed, tilt);
        }
    }


    void LaserTurret::onNXTUpdate(uint8_t /* laserPower */, int pan, uint8_t panSpeed, int tilt, uint8_t tiltSpeed)
    {
        m_Pan = pan;
        m_PanSpeed = panSpeed;
        m_Tilt = tilt;
        m_TiltSpeed = tiltSpeed;

        m_rtDispatcherQueue.TryEnqueue(
            Microsoft::UI::Dispatching::DispatcherQueuePriority::Normal,
            [this]
            {
                RaisePropertyChanged(L"");
            }
        );
    }
}
