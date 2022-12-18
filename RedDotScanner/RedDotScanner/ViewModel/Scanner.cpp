#include "pch.h"
#include "Scanner.h"
#include "Scanner.g.cpp"


namespace winrt::RedDotScanner::implementation
{
    Scanner::Scanner(Microsoft::UI::Dispatching::DispatcherQueue const& rtDispatcherQueue, RedDotScanner::WorldView const& rtWorldView)
        : m_Scanner(nullptr)
        , m_rtDispatcherQueue(rtDispatcherQueue)
        , m_rtiTriangulator(winrt::make_self<RedDotScanner::implementation::Triangulator>(rtDispatcherQueue, rtWorldView))
        , m_rtTriangulator(*m_rtiTriangulator)
        , m_rtiLaserTurret(winrt::make_self<RedDotScanner::implementation::LaserTurret>(rtDispatcherQueue, this))
        , m_rtLaserTurret(*m_rtiLaserTurret)
        , m_nSamplesU(5)
        , m_nSamplesV(5)
        , m_ProgressValue(0)
        , m_LaserPower(0)
        , m_pWorldView(winrt::get_self<WorldView>(rtWorldView))
    {
    }

    void Scanner::Close()
    {
        m_Scanner.reset();

        m_rtLaserTurret.Close();
        m_rtTriangulator.Close();
    }

    winrt::RedDotScanner::Triangulator Scanner::Triangulator()
    {
        return m_rtTriangulator;
    }

    winrt::RedDotScanner::LaserTurret Scanner::LaserTurret()
    {
        return m_rtLaserTurret;
    }

    bool Scanner::IsAvailable()
    {
        return m_Scanner != nullptr;
    }

    winrt::RedDotScanner::PanTilt Scanner::ScanAreaA()
    {
        return m_rtScanAreaA;
    }

    void Scanner::ScanAreaA(winrt::RedDotScanner::PanTilt const& value)
    {
        m_rtScanAreaA = value;
        RaisePropertyChanged(L"ScanAreaA");
    }

    winrt::RedDotScanner::PanTilt Scanner::ScanAreaB()
    {
        return m_rtScanAreaB;
    }

    void Scanner::ScanAreaB(winrt::RedDotScanner::PanTilt const& value)
    {
        m_rtScanAreaB = value;
        RaisePropertyChanged(L"ScanAreaB");
    }

    winrt::RedDotScanner::PanTilt Scanner::ScanAreaC()
    {
        return m_rtScanAreaC;
    }

    void Scanner::ScanAreaC(winrt::RedDotScanner::PanTilt const& value)
    {
        m_rtScanAreaC = value;
        RaisePropertyChanged(L"ScanAreaC");
    }

    winrt::RedDotScanner::PanTilt Scanner::ScanAreaD()
    {
        return m_rtScanAreaD;
    }

    void Scanner::ScanAreaD(winrt::RedDotScanner::PanTilt const& value)
    {
        m_rtScanAreaD = value;
        RaisePropertyChanged(L"ScanAreaD");
    }


    int32_t Scanner::SampleCountU()
    {
        return m_nSamplesU;
    }

    void Scanner::SampleCountU(int32_t value)
    {
        m_nSamplesU = value;
        RaisePropertyChanged(L"SampleCountU");
        RaisePropertyChanged(L"ProgressMax");
    }

    int32_t Scanner::SampleCountV()
    {
        return m_nSamplesV;
    }

    void Scanner::SampleCountV(int32_t value)
    {
        m_nSamplesV = value;
        RaisePropertyChanged(L"SampleCountV");
        RaisePropertyChanged(L"ProgressMax");
    }


    bool Scanner::IsScanning()
    {
        if (m_Scanner)
            return m_Scanner->isScanning();
        else
            return false;
    }

    void Scanner::StartScanning()
    {
        if (m_Scanner) {
            m_ProgressValue = 0;

            m_pWorldView->UpdateFromScanner({});

            m_Scanner->startScanning(
                scanner::PanTilt(m_rtScanAreaA.Pan(), m_rtScanAreaA.Tilt()),
                scanner::PanTilt(m_rtScanAreaB.Pan(), m_rtScanAreaB.Tilt()),
                scanner::PanTilt(m_rtScanAreaC.Pan(), m_rtScanAreaC.Tilt()),
                scanner::PanTilt(m_rtScanAreaD.Pan(), m_rtScanAreaD.Tilt()),
                m_nSamplesU,
                m_nSamplesV
            );

            RaisePropertyChanged(L"IsScanning");
            RaisePropertyChanged(L"IsStartButtonEnabled");
            RaisePropertyChanged(L"IsStopButtonEnabled");
            RaisePropertyChanged(L"ProgressValue");
        }
    }

    void Scanner::StopScanning()
    {
        if (m_Scanner) {
            m_Scanner->stopScanning();

            RaisePropertyChanged(L"IsScanning");
            RaisePropertyChanged(L"IsStartButtonEnabled");
            RaisePropertyChanged(L"IsStopButtonEnabled");
            RaisePropertyChanged(L"IsCaptureDataAvailable");
        }
    }

    void Scanner::SaveScanToObj(hstring const& rtFilename)
    {
        if (m_Scanner) {
            m_Scanner->saveToObj(winrt::to_string(rtFilename));
        }
    }


    bool Scanner::IsStartButtonEnabled()
    {
        return IsAvailable() && !IsScanning();
    }

    bool Scanner::IsStopButtonEnabled()
    {
        return IsAvailable() && IsScanning();
    }

    bool Scanner::IsCaptureDataAvailable()
    {
        return m_Scanner && m_Scanner->getScannedPoints().size() > 0;
    }


    int32_t Scanner::ProgressValue()
    {
        return m_ProgressValue;
    }

    int32_t Scanner::ProgressMax()
    {
        return m_nSamplesU * m_nSamplesV;
    }

    void Scanner::Save(hstring const& rtFilename)
    {
        if (m_Scanner)
            m_Scanner->save(to_string(rtFilename));
    }

    void Scanner::Load(hstring const& rtFilename)
    {
        if (m_Scanner) {
            m_Scanner->getTriangulator().clearCameras();
            m_rtiTriangulator->triangulatorUpdated();

            m_Scanner->load(to_string(rtFilename));

            m_rtiTriangulator->triangulatorUpdated();
            m_pWorldView->UpdateFromScanner(m_Scanner->getScannedPoints());

            SampleCountU(m_Scanner->getSamplesU());
            SampleCountV(m_Scanner->getSamplesV());

            RaisePropertyChanged(L"IsCaptureDataAvailable");
        }
    }

    winrt::RedDotScanner::BundleAdjustmentResult Scanner::RunBundleAdjustment()
    {
        double initReprojError = 0.0;
        double calibReprojError = 0.0;

        if (m_Scanner) {
            m_Scanner->runBundleAdjustment(initReprojError, calibReprojError);

            m_pWorldView->UpdateFromScanner(m_Scanner->getScannedPoints());
        }

        return winrt::RedDotScanner::BundleAdjustmentResult(initReprojError, calibReprojError);
    }

    void Scanner::DoLeveling()
    {
        // Turn off laser
        m_LaserPower = m_rtiLaserTurret->LaserPower();
        m_rtiLaserTurret->LaserPower(0);

        // Start leveling
        m_Leveler = std::make_shared<scanner::Leveler>(m_rtiTriangulator->getTriangulator());
        m_Leveler->start();
    }


    void Scanner::setLaserTurret(scanner::NXTLaserTurret* laserTurret)
    {
        m_Scanner.release();

        if (laserTurret) {
            m_Scanner = std::make_unique<scanner::Scanner>(m_rtiTriangulator->getTriangulator(), *laserTurret);
            m_Scanner->addEventHandler(this);
        }

        RaisePropertyChanged(L"IsAvailable");
        RaisePropertyChanged(L"IsStartButtonEnabled");
        RaisePropertyChanged(L"IsStopButtonEnabled");
    }

    void Scanner::onScanningProgress(int u, int v, const std::vector<scanner::ScannedPoint>& scannedPoints)
    {
        m_rtDispatcherQueue.TryEnqueue(
            Microsoft::UI::Dispatching::DispatcherQueuePriority::Normal,
            [this, u, v, scannedPoints]
            {
                m_ProgressValue = v * m_Scanner->getSamplesU() + u + 1;
                RaisePropertyChanged(L"ProgressValue");

                m_pWorldView->UpdateFromScanner(scannedPoints);
            }
        );
    }

    void Scanner::onScanningFinished()
    {
        m_rtDispatcherQueue.TryEnqueue(
            Microsoft::UI::Dispatching::DispatcherQueuePriority::Normal,
            [this]
            {
                RaisePropertyChanged(L"IsScanning");
                RaisePropertyChanged(L"IsStartButtonEnabled");
                RaisePropertyChanged(L"IsStopButtonEnabled");
            }
        );
    }


    void Scanner::onLevelingFinished()
    {
        // Update things
        m_rtDispatcherQueue.TryEnqueue(
            Microsoft::UI::Dispatching::DispatcherQueuePriority::Normal,
            [this]
            {
                m_rtiLaserTurret->LaserPower(m_LaserPower);
            }
        );

        m_Leveler.reset();
    }
}
