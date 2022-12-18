#pragma once

#include <DirectXMath.h>


namespace winrt::RedDotScanner::implementation
{

	class SimpleShader
	{
	public:
		struct Vertex
		{
			DirectX::XMFLOAT3 vPos;
		};

	public:
		struct ModelViewProjectionConstantBuffer
		{
			DirectX::XMFLOAT4X4 model;
			DirectX::XMFLOAT4X4 view;
			DirectX::XMFLOAT4X4 projection;

			ModelViewProjectionConstantBuffer()
			{
				DirectX::XMStoreFloat4x4(&model, DirectX::XMMatrixIdentity());
				DirectX::XMStoreFloat4x4(&view, DirectX::XMMatrixIdentity());
				DirectX::XMStoreFloat4x4(&projection, DirectX::XMMatrixIdentity());
			}
		};
		static_assert((sizeof(ModelViewProjectionConstantBuffer) % 16) == 0, "Constant Buffer size must be 16-byte aligned");

		struct SimpleParamsConstantBuffer
		{
			DirectX::XMFLOAT4 color;

			SimpleParamsConstantBuffer()
				: color(0.0f, 0.0f, 0.0f, 1.0f)
			{
			}
		};
		static_assert((sizeof(SimpleParamsConstantBuffer) % 16) == 0, "Constant Buffer size must be 16-byte aligned");

		struct ConstantBuffers
		{
			ConstantBuffers() = default;
			ConstantBuffers(const winrt::com_ptr<ID3D11Device>& device);

			ModelViewProjectionConstantBuffer vsMVP;
			SimpleParamsConstantBuffer psColor;

			void Apply(winrt::com_ptr<ID3D11DeviceContext>& context) const;

		private:
			friend class SimpleShader;

			winrt::com_ptr<ID3D11Buffer> m_d3dVSMVP;
			winrt::com_ptr<ID3D11Buffer> m_d3dPSColor;
		};

	public:
		SimpleShader() = default;
		SimpleShader(winrt::com_ptr<ID3D11Device>& device);

		void Apply(winrt::com_ptr<ID3D11DeviceContext>& context) const;

	private:
		winrt::com_ptr<ID3D11VertexShader> m_VertexShader;
		winrt::com_ptr<ID3D11PixelShader> m_PixelShader;
		winrt::com_ptr<ID3D11InputLayout> m_InputLayout;
	};

}