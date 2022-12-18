#include "pch.h"
#include "Direct3DPanel.h"
#if __has_include("Direct3DPanel.g.cpp")
#include "Direct3DPanel.g.cpp"
#endif

#undef max

#include <algorithm>


using namespace winrt;
using namespace Microsoft::UI::Xaml;


namespace winrt::RedDotScanner::implementation
{
    Direct3DPanel::Direct3DPanel()
        : m_d3dFeatureLevel(D3D_FEATURE_LEVEL_9_1)
        , m_Width(1.0f)
        , m_Height(1.0f)
        , m_CompositionScaleX(1.0f)
        , m_CompositionScaleY(1.0f)
        , m_RenderTargetWidth(1)
        , m_RenderTargetHeight(1)
    {
        auto unknown = this->try_as<IUnknown>();
        unknown->QueryInterface<ISwapChainPanelNative>(m_SwapChainPanelNative.put());

        SizeChanged(Microsoft::UI::Xaml::SizeChangedEventHandler(this, &Direct3DPanel::OnSizeChanged));
        CompositionScaleChanged(Windows::Foundation::TypedEventHandler<Microsoft::UI::Xaml::Controls::SwapChainPanel,
            winrt::Windows::Foundation::IInspectable>(this, &Direct3DPanel::OnCompositionScaleChanged)); // Why can't things be easy with WinRT?

        CreateDeviceResources();
    }

    void Direct3DPanel::OnSizeChanged(Windows::Foundation::IInspectable const& /*sender*/, Microsoft::UI::Xaml::SizeChangedEventArgs const& e)
    {
        if (m_Width != e.NewSize().Width || m_Height != e.NewSize().Height) {
            m_Width = std::max(e.NewSize().Width, 1.0f);
            m_Height = std::max(e.NewSize().Height, 1.0f);

            CreateSizeDependentResources();
        }
    }

    void Direct3DPanel::OnCompositionScaleChanged(Microsoft::UI::Xaml::Controls::SwapChainPanel const& /*sender*/, Windows::Foundation::IInspectable const& /*args*/)
    {
        if (m_CompositionScaleX != CompositionScaleX() || m_CompositionScaleY != CompositionScaleY()) {
            m_CompositionScaleX = CompositionScaleX();
            m_CompositionScaleY = CompositionScaleY();

            CreateSizeDependentResources();
        }
    }

    void Direct3DPanel::CreateDeviceResources()
    {
        UINT deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(DEBUG) || defined(_DEBUG)
        deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_9_1
        };

        winrt::check_hresult(D3D11CreateDevice(
            /* pAdapter */ nullptr,
            /* DriverType */ D3D_DRIVER_TYPE_HARDWARE,
            /* Software */ nullptr,
            /* Flags */ deviceFlags,
            /* pFeatureLevels */ featureLevels,
            /* FeatureLevels */ ARRAYSIZE(featureLevels),
            /* SDKVersion */ D3D11_SDK_VERSION,
            /* ppDevice */ m_d3dDevice.put(),
            /* pFeatureLevel */ &m_d3dFeatureLevel,
            /* ppImmediateContext */ m_d3dContext.put()
        ));
    }

    void Direct3DPanel::CreateSizeDependentResources()
    {
        // Release back buffer
        m_d3dContext->OMSetRenderTargets(0, nullptr, nullptr);
        m_d3dContext->Flush();

        m_RenderTargetView = nullptr;

        // Resize everything
        m_RenderTargetWidth = static_cast<unsigned int>(m_Width * m_CompositionScaleX);
        m_RenderTargetHeight = static_cast<unsigned int>(m_Height * m_CompositionScaleY);

        if (m_SwapChain != nullptr)
        {
            HRESULT hr = m_SwapChain->ResizeBuffers(
                2,
                m_RenderTargetWidth,
                m_RenderTargetHeight,
                DXGI_FORMAT_B8G8R8A8_UNORM,
                0
            );

            if (hr == DXGI_ERROR_DEVICE_RESET || hr == DXGI_ERROR_DEVICE_REMOVED)
            {
                OnDeviceLost();
                return;
            }
            else
            {
                winrt::check_hresult(hr);
            }
        }
        else
        {
            winrt::com_ptr<IDXGIDevice> dxgiDevice = m_d3dDevice.as<IDXGIDevice>();
            winrt::com_ptr<IDXGIAdapter> dxgiAdapter;
            winrt::check_hresult(dxgiDevice->GetAdapter(dxgiAdapter.put()));

            winrt::com_ptr<IDXGIFactory2> dxgiFactory;
            winrt::check_hresult(dxgiAdapter->GetParent(IID_PPV_ARGS(dxgiFactory.put())));

            DXGI_SWAP_CHAIN_DESC1 desc = { 0 };
            desc.Width = m_RenderTargetWidth;
            desc.Height = m_RenderTargetHeight;
            desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
            desc.Stereo = false;
            desc.SampleDesc.Count = 1;
            desc.SampleDesc.Quality = 0;
            desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            desc.BufferCount = 2;
            desc.Scaling = DXGI_SCALING_STRETCH;
            desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
            desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
            desc.Flags = 0;

            winrt::com_ptr<IDXGISwapChain1> swapChain1;
            winrt::check_hresult(dxgiFactory->CreateSwapChainForComposition(
                /* pDevice */ m_d3dDevice.get(),
                /* pDesc */ &desc,
                /* pRestrictToOutput */ nullptr,
                /* ppSwapChain */ swapChain1.put()
            ));

            m_SwapChain = swapChain1.as<IDXGISwapChain2>();

            m_SwapChainPanelNative->SetSwapChain(m_SwapChain.get());
        }

        winrt::com_ptr<ID3D11Texture2D> backBuffer;
        m_SwapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.put()));
        m_d3dDevice->CreateRenderTargetView(backBuffer.get(), nullptr, m_RenderTargetView.put());


        DXGI_MATRIX_3X2_F inverseScale = { 0 };
        inverseScale._11 = 1.0f / m_CompositionScaleX;
        inverseScale._22 = 1.0f / m_CompositionScaleY;
        m_SwapChain->SetMatrixTransform(&inverseScale);
    }

    void Direct3DPanel::OnDeviceLost()
    {
        // Release everything
        m_d3dContext->OMSetRenderTargets(0, nullptr, nullptr);
        m_d3dContext->Flush();

        m_SwapChainPanelNative->SetSwapChain(nullptr);

        m_RenderTargetView = nullptr;

        m_SwapChain = nullptr;
        m_d3dContext = nullptr;
        m_d3dDevice = nullptr;
        
        // Recreate everything
        CreateDeviceResources();
        CreateSizeDependentResources();
    }

    void Direct3DPanel::Present()
    {
        HRESULT hr = m_SwapChain->Present(1, 0);

        if (hr == DXGI_ERROR_DEVICE_RESET || hr == DXGI_ERROR_DEVICE_REMOVED)
        {
            OnDeviceLost();
        }
        else
        {
            winrt::check_hresult(hr);
        }
    }
}
