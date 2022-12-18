#pragma once
#include "BundleAdjustmentResult.g.h"


namespace winrt::RedDotScanner::implementation
{
    struct BundleAdjustmentResult : BundleAdjustmentResultT<BundleAdjustmentResult>
    {
        BundleAdjustmentResult();
        BundleAdjustmentResult(double initReprojError, double calibReprojError);

        double InitialReprojectionError();
        double CalibrationReprojectionError();

    private:
        double m_InitReprojError, m_CalibReprojError;
    };
}
namespace winrt::RedDotScanner::factory_implementation
{
    struct BundleAdjustmentResult : BundleAdjustmentResultT<BundleAdjustmentResult, implementation::BundleAdjustmentResult>
    {
    };
}
