#pragma once

#include <cassert>

#include <vector>

#include <winrt/base.h>
#include <d3d11.h>


namespace winrt::RedDotScanner::implementation
{

	template <class T>
	struct IndexBufferFormat { };

	template <>
	struct IndexBufferFormat<unsigned int> { static const DXGI_FORMAT Type = DXGI_FORMAT_R32_UINT; };

	template <>
	struct IndexBufferFormat<unsigned short> { static const DXGI_FORMAT Type = DXGI_FORMAT_R16_UINT; };


	template <class t_vertex>
	class Mesh
	{
	public:
		Mesh() = default;
		Mesh(winrt::com_ptr<ID3D11Device>& device, D3D11_PRIMITIVE_TOPOLOGY primitiveTopology, const std::vector<t_vertex>& vertexes);

		template <class t_index>
		Mesh(winrt::com_ptr<ID3D11Device>& device, D3D11_PRIMITIVE_TOPOLOGY primitiveTopology, const std::vector<t_vertex>& vertexes, const std::vector<t_index>& indexes);

		void Draw(winrt::com_ptr<ID3D11DeviceContext>& context) const;

	private:
		D3D11_PRIMITIVE_TOPOLOGY m_PrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
		winrt::com_ptr<ID3D11Buffer> m_VertexBuffer;
		winrt::com_ptr<ID3D11Buffer> m_IndexBuffer;
		DXGI_FORMAT m_IndexBufferFormat = DXGI_FORMAT_UNKNOWN;
		unsigned int m_Count = 0;
	};



	template <class t_vertex>
	Mesh<t_vertex>::Mesh(winrt::com_ptr<ID3D11Device>& device, D3D11_PRIMITIVE_TOPOLOGY primitiveTopology, const std::vector<t_vertex>& vertexes)
		: m_PrimitiveTopology(primitiveTopology)
		, m_Count(vertexes.size())
	{
		D3D11_SUBRESOURCE_DATA vData = { 0 };
		vData.pSysMem = vertexes.data();

		CD3D11_BUFFER_DESC vDesc(
			vertexes.size() * sizeof(t_vertex),
			D3D11_BIND_VERTEX_BUFFER
		);
		winrt::check_hresult(device->CreateBuffer(&vDesc, &vData, m_VertexBuffer.put()));
	}

	template <class t_vertex>
	template <class t_index>
	Mesh<t_vertex>::Mesh(winrt::com_ptr<ID3D11Device>& device, D3D11_PRIMITIVE_TOPOLOGY primitiveTopology, const std::vector<t_vertex>& vertexes, const std::vector<t_index>& indexes)
		: m_PrimitiveTopology(primitiveTopology)
		, m_Count(indexes.size())
	{
		CD3D11_BUFFER_DESC vDesc(
			vertexes.size() * sizeof(t_vertex),
			D3D11_BIND_VERTEX_BUFFER
		);

		D3D11_SUBRESOURCE_DATA vData = { 0 };
		vData.pSysMem = vertexes.data();

		winrt::check_hresult(device->CreateBuffer(&vDesc, &vData, m_VertexBuffer.put()));


		CD3D11_BUFFER_DESC iDesc(
			indexes.size() * sizeof(t_index),
			D3D11_BIND_INDEX_BUFFER
		);

		D3D11_SUBRESOURCE_DATA iData = { 0 };
		iData.pSysMem = indexes.data();

		winrt::check_hresult(device->CreateBuffer(&iDesc, &iData, m_IndexBuffer.put()));


		m_IndexBufferFormat = IndexBufferFormat<t_index>::Type;
	}

	template <class t_vertex>
	void Mesh<t_vertex>::Draw(winrt::com_ptr<ID3D11DeviceContext>& context) const
	{
		assert(m_VertexBuffer != nullptr);


		context->IASetPrimitiveTopology(m_PrimitiveTopology);

		UINT stride = sizeof(t_vertex);
		UINT offset = 0;
		ID3D11Buffer* vertexBuffers[] = { m_VertexBuffer.get() };
		context->IASetVertexBuffers(0, 1, vertexBuffers, &stride, &offset);

		if (m_IndexBuffer != nullptr)
		{
			context->IASetIndexBuffer(m_IndexBuffer.get(), m_IndexBufferFormat, 0);

			context->DrawIndexed(m_Count, 0, 0);
		}
		else
		{
			context->Draw(m_Count, 0);
		}
	}

}