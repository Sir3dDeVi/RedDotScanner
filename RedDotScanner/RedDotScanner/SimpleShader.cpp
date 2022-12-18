#include "pch.h"
#include "SimpleShader.h"

#include <cassert>

#include <Utils.h>


namespace winrt::RedDotScanner::implementation
{

	namespace SimpleShaderData
	{
		const char* szVertexShaderFilename = "SimpleVertexShader.cso";
		const char* szPixelShaderFilename = "SimplePixelShader.cso";
		const D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};
	}


	SimpleShader::SimpleShader(winrt::com_ptr<ID3D11Device>& device)
	{
		auto vsBytecode = utils::readFile(SimpleShaderData::szVertexShaderFilename);
		auto psBytecode = utils::readFile(SimpleShaderData::szPixelShaderFilename);

		winrt::check_hresult(device->CreateVertexShader(vsBytecode.data(), vsBytecode.size(), nullptr, m_VertexShader.put()));
		winrt::check_hresult(device->CreatePixelShader(psBytecode.data(), psBytecode.size(), nullptr, m_PixelShader.put()));

		device->CreateInputLayout(SimpleShaderData::inputLayoutDesc, ARRAYSIZE(SimpleShaderData::inputLayoutDesc), vsBytecode.data(), vsBytecode.size(), m_InputLayout.put());
	}

	void SimpleShader::Apply(winrt::com_ptr<ID3D11DeviceContext>& context) const
	{
		assert(m_InputLayout != nullptr);
		assert(m_VertexShader != nullptr);
		assert(m_PixelShader != nullptr);

		context->IASetInputLayout(m_InputLayout.get());
		context->VSSetShader(m_VertexShader.get(), nullptr, 0);
		context->PSSetShader(m_PixelShader.get(), nullptr, 0);
	}


	// SimpleShader::ConstantBuffers ///////////////////////////////////////////////


	SimpleShader::ConstantBuffers::ConstantBuffers(const winrt::com_ptr<ID3D11Device>& device)
	{
		CD3D11_BUFFER_DESC vsMVPDesc(
			sizeof(ModelViewProjectionConstantBuffer),
			D3D11_BIND_CONSTANT_BUFFER
		);
		device->CreateBuffer(&vsMVPDesc, nullptr, m_d3dVSMVP.put());

		CD3D11_BUFFER_DESC psColorDesc(
			sizeof(SimpleParamsConstantBuffer),
			D3D11_BIND_CONSTANT_BUFFER
		);
		device->CreateBuffer(&psColorDesc, nullptr, m_d3dPSColor.put());
	}

	void SimpleShader::ConstantBuffers::Apply(winrt::com_ptr<ID3D11DeviceContext>& context) const
	{
		assert(m_d3dVSMVP != nullptr);
		assert(m_d3dPSColor != nullptr);


		context->UpdateSubresource(m_d3dVSMVP.get(), 0, nullptr, &vsMVP, 0, 0);
		context->UpdateSubresource(m_d3dPSColor.get(), 0, nullptr, &psColor, 0, 0);


		ID3D11Buffer* vsCBs[] = { m_d3dVSMVP.get() };
		context->VSSetConstantBuffers(0, 1, vsCBs);

		ID3D11Buffer* psCBs[] = { m_d3dPSColor.get() };
		context->PSSetConstantBuffers(0, 1, psCBs);
	}

}