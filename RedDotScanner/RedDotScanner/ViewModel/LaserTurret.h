#pragma once
#include "LaserTurret.g.h"

#include "ViewModel/NotifyPropretyChangedImpl.h"

#include <scanner/NXTLaserTurret.h>
#include <scanner/Scanner.h>


namespace winrt::RedDotScanner::implementation
{
    struct Scanner;

    struct LaserTurret
        : LaserTurretT<LaserTurret>
        , NotifyPropertyChangedImplT<LaserTurret>
        , scanner::INXTLaserTurretCallback
    {
        LaserTurret(Microsoft::UI::Dispatching::DispatcherQueue const& rtDispatcherQueue);
        LaserTurret(Microsoft::UI::Dispatching::DispatcherQueue const& rtDispatcherQueue, Scanner* prtiScanner);

        void RefreshNXTDevices();
        winrt::Windows::Foundation::Collections::IObservableVector<winrt::RedDotScanner::NXTDeviceInfo> NXTDevices();
        winrt::RedDotScanner::NXTDeviceInfo CurrentNXTDevice();
        void CurrentNXTDevice(winrt::RedDotScanner::NXTDeviceInfo const& value);

        bool IsAvailable() const;
        uint8_t LaserPower();
        void LaserPower(uint8_t value);
        int32_t Pan();
        uint8_t PanSpeed();
        int32_t Tilt();
        uint8_t TiltSpeed();
        void TiltRotate(int16_t speed);
        void PanRotate(int16_t speed);
        void RotateTo(int32_t pan, uint8_t panSpeed, int32_t tilt, uint8_t tiltSpeed);

        // IClosable
        void Close();

    public:
        // INXTLaserTurretCallback
        virtual void onNXTUpdate(uint8_t laserPower, int pan, uint8_t panSpeed, int tilt, uint8_t tiltSpeed) override;

    private:
        std::unique_ptr<scanner::NXTLaserTurret> m_NXTLaserTurret;

        Microsoft::UI::Dispatching::DispatcherQueue m_rtDispatcherQueue;

        Scanner* m_prtiScanner;

        Windows::Foundation::Collections::IObservableVector<RedDotScanner::NXTDeviceInfo> m_rtNXTDevices;
        RedDotScanner::NXTDeviceInfo m_rtCurrentNXTDeVice{ nullptr };

        uint8_t m_Laser;
        int32_t m_Pan, m_Tilt;
        uint8_t m_PanSpeed, m_TiltSpeed;
    };
}
namespace winrt::RedDotScanner::factory_implementation
{
    struct LaserTurret : LaserTurretT<LaserTurret, implementation::LaserTurret>
    {
    };
}