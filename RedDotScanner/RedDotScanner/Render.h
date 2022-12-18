#pragma once

#include <winrt/base.h>
#include <d3d11.h>

#include "Mesh.h"


namespace winrt::RedDotScanner::implementation
{

	template <class t_shader>
	void RenderMesh(
		winrt::com_ptr<ID3D11DeviceContext>& context,
		const t_shader& shader,
		const typename t_shader::ConstantBuffers& cbs,
		const Mesh<typename t_shader::Vertex>& mesh
	)
	{
		shader.Apply(context);
		cbs.Apply(context);
		mesh.Draw(context);
	}

}