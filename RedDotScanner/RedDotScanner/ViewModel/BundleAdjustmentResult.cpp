#include "pch.h"
#include "BundleAdjustmentResult.h"
#include "BundleAdjustmentResult.g.cpp"


namespace winrt::RedDotScanner::implementation
{
    BundleAdjustmentResult::BundleAdjustmentResult()
        : m_InitReprojError(0.0)
        , m_CalibReprojError(0.0)
    {
    }

    BundleAdjustmentResult::BundleAdjustmentResult(double initReprojError, double calibReprojError)
        : m_InitReprojError(initReprojError)
        , m_CalibReprojError(calibReprojError)
    {
    }

    double BundleAdjustmentResult::InitialReprojectionError()
    {
        return m_InitReprojError;
    }

    double BundleAdjustmentResult::CalibrationReprojectionError()
    {
        return m_CalibReprojError;
    }
}
