#pragma once


#include <winrt/base.h>
#include <d3d11.h>
#include <DirectXMath.h>

#include "Mesh.h"
#include "Render.h"


namespace winrt::RedDotScanner::implementation
{

	template <class t_shader>
	struct Object : public t_shader::ConstantBuffers
	{
		Object() = default;

		Object(winrt::com_ptr<ID3D11Device>& device, D3D11_PRIMITIVE_TOPOLOGY primitiveTopology, const std::vector<typename t_shader::Vertex>& vertexes);

		template <class t_index>
		Object(winrt::com_ptr<ID3D11Device>& device, D3D11_PRIMITIVE_TOPOLOGY primitiveTopology, const std::vector<typename t_shader::Vertex>& vertexes, const std::vector<t_index>& indexes);

		void Render(winrt::com_ptr<ID3D11DeviceContext>& context, const t_shader& shader, const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& projection);

	private:
		Mesh<typename t_shader::Vertex> m_Mesh;
	};



	template <class t_shader>
	Object<t_shader>::Object(winrt::com_ptr<ID3D11Device>& device, D3D11_PRIMITIVE_TOPOLOGY primitiveTopology, const std::vector<typename t_shader::Vertex>& vertexes)
		: t_shader::ConstantBuffers(device)
		, m_Mesh(device, primitiveTopology, vertexes)
	{
	}

	template <class t_shader>
	template <class t_index>
	Object<t_shader>::Object(winrt::com_ptr<ID3D11Device>& device, D3D11_PRIMITIVE_TOPOLOGY primitiveTopology, const std::vector<typename t_shader::Vertex>& vertexes, const std::vector<t_index>& indexes)
		: t_shader::ConstantBuffers(device)
		, m_Mesh(device, primitiveTopology, vertexes, indexes)
	{
	}

	template <class t_shader>
	void Object<t_shader>::Render(winrt::com_ptr<ID3D11DeviceContext>& context, const t_shader& shader, const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& projection)
	{
		this->vsMVP.view = view;
		this->vsMVP.projection = projection;

		RenderMesh(context, shader, *this, m_Mesh);
	}

}