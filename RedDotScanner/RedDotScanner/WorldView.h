#pragma once

#include "WorldView.g.h"
#include "Direct3DPanel.h"

#include <DirectXMath.h>

#include <mutex>

#include <scanner/Triangulator.h>
#include <scanner/Scanner.h>

#include "SimpleShader.h"
#include "Object.h"


namespace winrt::RedDotScanner::implementation
{
    struct WorldView : WorldViewT<WorldView, RedDotScanner::implementation::Direct3DPanel>
    {
        WorldView();

        void Render();

        void UpdateFromTriangulator(const std::optional<cv::Point3f>& point, const std::map<const scanner::Camera*, std::optional<scanner::RayInfo>>& rays);
        void UpdateFromScanner(const std::vector<scanner::ScannedPoint>& scannedPoints);


    protected:
        virtual void CreateDeviceResources() override;
        virtual void CreateSizeDependentResources() override;

        void OnPointerMoved(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
        void OnPointerPressed(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
        void OnPointerReleased(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
        void OnPointerWheelChanged(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);

    private:
        enum class CameraManipulation { None, Orbit, Zoom, Pan };

    private:
        winrt::com_ptr<ID3D11DepthStencilView> m_DepthStencilView;

        DirectX::XMFLOAT4X4 m_CameraProjection;
        CameraManipulation m_CameraManipulation;
        float m_CameraDistance, m_CameraPhi, m_CameraTheta;
        DirectX::XMFLOAT3 m_CameraTarget;
        DirectX::XMFLOAT2 m_LastMoustPos;


        SimpleShader m_SimpleShader;

        winrt::com_ptr<ID3D11DepthStencilState> m_DSSLines;
        winrt::com_ptr<ID3D11RasterizerState> m_RSSLines;
        winrt::com_ptr<ID3D11BlendState> m_BSProjectionLines;

        Object<SimpleShader> m_GridAxisXPos, m_GridAxisXNeg;
        Object<SimpleShader> m_GridAxisYPos, m_GridAxisYNeg;
        Object<SimpleShader> m_GridAxisZ;
        Object<SimpleShader> m_GridMeters;
        Object<SimpleShader> m_GridInner;

        Object<SimpleShader> m_Line;
        Object<SimpleShader> m_Sphere;

        std::optional<cv::Point3f> m_Point;
        std::map<const scanner::Camera*, std::optional<scanner::RayInfo>> m_Rays;
        std::vector<scanner::ScannedPoint> m_ScannedPoints;

        void DrawLine(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b, const DirectX::XMFLOAT4& color, const DirectX::XMFLOAT4X4& view);
        void DrawLine(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b, const DirectX::XMFLOAT4& color, const DirectX::XMFLOAT4X4& model, const DirectX::XMFLOAT4X4& view);
    };
}
namespace winrt::RedDotScanner::factory_implementation
{
    struct WorldView : WorldViewT<WorldView, implementation::WorldView>
    {
    };
}
