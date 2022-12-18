#pragma once

#include "winrt/Microsoft.UI.Xaml.h"
#include "winrt/Microsoft.UI.Xaml.Markup.h"
#include "winrt/Microsoft.UI.Xaml.Controls.Primitives.h"
#include "Direct3DPanel.g.h"


namespace winrt::RedDotScanner::implementation
{
    struct Direct3DPanel : Direct3DPanelT<Direct3DPanel>
    {
        Direct3DPanel();

    protected:
        virtual void OnSizeChanged(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::SizeChangedEventArgs const& e);
        virtual void OnCompositionScaleChanged(Microsoft::UI::Xaml::Controls::SwapChainPanel const& sender, Windows::Foundation::IInspectable const& args);

        virtual void CreateDeviceResources();
        virtual void CreateSizeDependentResources();
        virtual void OnDeviceLost();

        virtual void Present();

    protected:
        winrt::com_ptr<ISwapChainPanelNative> m_SwapChainPanelNative;

        winrt::com_ptr<ID3D11Device> m_d3dDevice;
        winrt::com_ptr<ID3D11DeviceContext> m_d3dContext;
        D3D_FEATURE_LEVEL m_d3dFeatureLevel;
        winrt::com_ptr<IDXGISwapChain2> m_SwapChain;

        winrt::com_ptr<ID3D11RenderTargetView> m_RenderTargetView;

        float m_Width, m_Height;
        float m_CompositionScaleX, m_CompositionScaleY;
        unsigned int m_RenderTargetWidth, m_RenderTargetHeight;
    };
}
