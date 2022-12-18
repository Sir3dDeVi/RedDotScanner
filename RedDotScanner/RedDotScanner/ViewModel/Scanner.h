#pragma once
#include "Scanner.g.h"

#include "ViewModel/NotifyPropretyChangedImpl.h"

#include "ViewModel/LaserTurret.h"
#include "ViewModel/PanTilt.h"
#include "ViewModel/Triangulator.h"
#include "WorldView.h"

#include <scanner/Scanner.h>
#include <scanner/Leveler.h>


namespace winrt::RedDotScanner::implementation
{
    struct Scanner
        : ScannerT<Scanner>
        , NotifyPropertyChangedImplT<Scanner>
        , scanner::IScannerCallback
        , scanner::ILevelerCallback
    {
        Scanner(Microsoft::UI::Dispatching::DispatcherQueue const& rtDispatcherQueue, RedDotScanner::WorldView const& rtWorldView);

        winrt::RedDotScanner::Triangulator Triangulator();
        winrt::RedDotScanner::LaserTurret LaserTurret();

        bool IsAvailable();


        winrt::RedDotScanner::PanTilt ScanAreaA();
        void ScanAreaA(winrt::RedDotScanner::PanTilt const& value);

        winrt::RedDotScanner::PanTilt ScanAreaB();
        void ScanAreaB(winrt::RedDotScanner::PanTilt const& value);

        winrt::RedDotScanner::PanTilt ScanAreaC();
        void ScanAreaC(winrt::RedDotScanner::PanTilt const& value);

        winrt::RedDotScanner::PanTilt ScanAreaD();
        void ScanAreaD(winrt::RedDotScanner::PanTilt const& value);

        int32_t SampleCountU();
        void SampleCountU(int32_t value);

        int32_t SampleCountV();
        void SampleCountV(int32_t value);


        bool IsScanning();
        void StartScanning();
        void StopScanning();
        void SaveScanToObj(hstring const& rtFilename);

        bool IsStartButtonEnabled();
        bool IsStopButtonEnabled();
        bool IsCaptureDataAvailable();


        int32_t ProgressValue();
        int32_t ProgressMax();

        void Save(hstring const& rtFilename);
        void Load(hstring const& rtFilename);

        winrt::RedDotScanner::BundleAdjustmentResult RunBundleAdjustment();

        void DoLeveling();

        // IClosable
        void Close();

    public:
        void setLaserTurret(scanner::NXTLaserTurret* laserTurret);

    public: // scanner::IScannerCallback
        virtual void onScanningProgress(int u, int v, const std::vector<scanner::ScannedPoint>& scannedPoints) override;
        virtual void onScanningFinished() override;

    public: // ILevelerCallback
        virtual void onLevelingFinished() override;

    private:
        std::unique_ptr<scanner::Scanner> m_Scanner;

        Microsoft::UI::Dispatching::DispatcherQueue m_rtDispatcherQueue;

        winrt::com_ptr<RedDotScanner::implementation::Triangulator> m_rtiTriangulator;
        RedDotScanner::Triangulator m_rtTriangulator;
        winrt::com_ptr<RedDotScanner::implementation::LaserTurret> m_rtiLaserTurret;
        RedDotScanner::LaserTurret m_rtLaserTurret;

        RedDotScanner::PanTilt m_rtScanAreaA, m_rtScanAreaB, m_rtScanAreaC, m_rtScanAreaD;
        int32_t m_nSamplesU;
        int32_t m_nSamplesV;

        std::mutex m_Mutex;
        int32_t m_ProgressValue;

        uint8_t m_LaserPower;
        std::shared_ptr<scanner::Leveler> m_Leveler;

        WorldView* m_pWorldView;
    };
}
namespace winrt::RedDotScanner::factory_implementation
{
    struct Scanner : ScannerT<Scanner, implementation::Scanner>
    {
    };
}