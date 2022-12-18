#include "pch.h"
#include "WorldView.h"
#include "WorldView.g.cpp"

#include <Utils.h>

#include "Render.h"


using namespace winrt;
using namespace Microsoft::UI::Xaml;


namespace winrt::RedDotScanner::implementation
{

	WorldView::WorldView()
		: m_CameraManipulation(CameraManipulation::None)
		, m_CameraDistance(1.5f)
		, m_CameraPhi(195.0f)
		, m_CameraTheta(45.0f)
		, m_CameraTarget(0.0f, 0.0f, 0.0f)
	{
		CreateDeviceResources();

		this->PointerMoved(Microsoft::UI::Xaml::Input::PointerEventHandler(this, &WorldView::OnPointerMoved));
		this->PointerPressed(Microsoft::UI::Xaml::Input::PointerEventHandler(this, &WorldView::OnPointerPressed));
		this->PointerReleased(Microsoft::UI::Xaml::Input::PointerEventHandler(this, &WorldView::OnPointerReleased));
		this->PointerExited(Microsoft::UI::Xaml::Input::PointerEventHandler(this, &WorldView::OnPointerReleased));
		this->PointerWheelChanged(Microsoft::UI::Xaml::Input::PointerEventHandler(this, &WorldView::OnPointerWheelChanged));
	}

	void WorldView::CreateDeviceResources()
	{
		// This is a bit hacky... So, WorldView::ctor() implicitly calls
		// Direct3DPanel::ctor(), which directly calls CreateDeviceResources(). However,
		// since at this point the base class is still being constructed, the vtable is not
		// updated, so WorldView::CreateDeviceResources() is not called polymorphically.
		// That's why I call it explicitly from WorldView::ctor().
		//
		// However, when the device is lost CreateDeviceResources() will be called to
		// create a new, and now everything works as expected.
		//
		// So that's why here is this check:
		// - to skip recreating the device during construction,
		// - but to create it OnDeviceLost(), because it nulls out m_d3dDevice.
		if (m_d3dDevice == nullptr)
			Direct3DPanel::CreateDeviceResources();


		m_SimpleShader = SimpleShader(m_d3dDevice);

		
		CD3D11_DEPTH_STENCIL_DESC dssLines(CD3D11_DEFAULT{});
		dssLines.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		m_d3dDevice->CreateDepthStencilState(&dssLines, m_DSSLines.put());

		CD3D11_RASTERIZER_DESC rssLines(CD3D11_DEFAULT{});
		rssLines.CullMode = D3D11_CULL_NONE;
		m_d3dDevice->CreateRasterizerState(&rssLines, m_RSSLines.put());

		CD3D11_BLEND_DESC bsProjectionLines(CD3D11_DEFAULT{});
		bsProjectionLines.IndependentBlendEnable = FALSE;
		bsProjectionLines.RenderTarget[0].BlendEnable = TRUE;
		bsProjectionLines.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		bsProjectionLines.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		bsProjectionLines.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		bsProjectionLines.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
		bsProjectionLines.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		bsProjectionLines.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		bsProjectionLines.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		m_d3dDevice->CreateBlendState(&bsProjectionLines, m_BSProjectionLines.put());

		{
			m_GridAxisXPos = Object<SimpleShader>(m_d3dDevice, D3D11_PRIMITIVE_TOPOLOGY_LINELIST, {
				{ DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f) }, { DirectX::XMFLOAT3(5.0f,  0.0f, 0.0f) }
			});
			m_GridAxisXPos.psColor.color = DirectX::XMFLOAT4(1.0f, 0.2f, 0.2f, 1.0f);

			m_GridAxisXNeg = Object<SimpleShader>(m_d3dDevice, D3D11_PRIMITIVE_TOPOLOGY_LINELIST, {
				{ DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f) }, { DirectX::XMFLOAT3(-5.0f,  0.0f, 0.0f) }
			});
			m_GridAxisXNeg.psColor.color = DirectX::XMFLOAT4(0.7f, 0.2f, 0.2f, 1.0f);

			m_GridAxisYPos = Object<SimpleShader>(m_d3dDevice, D3D11_PRIMITIVE_TOPOLOGY_LINELIST, {
				{ DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f) }, { DirectX::XMFLOAT3(0.0f,  5.0f, 0.0f) }
			});
			m_GridAxisYPos.psColor.color = DirectX::XMFLOAT4(0.0f, 0.8f, 0.0f, 1.0f);

			m_GridAxisYNeg = Object<SimpleShader>(m_d3dDevice, D3D11_PRIMITIVE_TOPOLOGY_LINELIST, {
				{ DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f) }, { DirectX::XMFLOAT3(0.0f, -5.0f, 0.0f) }
			});
			m_GridAxisYNeg.psColor.color = DirectX::XMFLOAT4(0.2f, 0.6f, 0.2f, 1.0f);

			m_GridAxisZ = Object<SimpleShader>(m_d3dDevice, D3D11_PRIMITIVE_TOPOLOGY_LINELIST, {
				{ DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f) }, { DirectX::XMFLOAT3(0.0f, 0.0f, 0.1f) }
			});
			m_GridAxisZ.psColor.color = DirectX::XMFLOAT4(0.2f, 0.2f, 1.0f, 1.0f);
		}

		{
			std::vector<SimpleShader::Vertex> vertexes;
			for (int y = -5; y <= 5; ++y) {
				if (y != 0) {
					vertexes.emplace_back(SimpleShader::Vertex{ DirectX::XMFLOAT3(-5.0f, static_cast<float>(y), 0.0f) });
					vertexes.emplace_back(SimpleShader::Vertex{ DirectX::XMFLOAT3( 5.0f, static_cast<float>(y), 0.0f) });
				}
			}
			for (int x = -5; x <= 5; ++x) {
				if (x != 0) {
					vertexes.emplace_back(SimpleShader::Vertex{ DirectX::XMFLOAT3(static_cast<float>(x), -5.0f, 0.0f) });
					vertexes.emplace_back(SimpleShader::Vertex{ DirectX::XMFLOAT3(static_cast<float>(x),  5.0f, 0.0f) });
				}
			}

			m_GridMeters = Object<SimpleShader>(m_d3dDevice, D3D11_PRIMITIVE_TOPOLOGY_LINELIST, vertexes);
			m_GridMeters.psColor.color = DirectX::XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
		}

		{
			std::vector<SimpleShader::Vertex> vertexes;
			for (int y = -50; y <= 50; ++y) {
				if (y % 10 != 0) {
					vertexes.emplace_back(SimpleShader::Vertex{ DirectX::XMFLOAT3(-5.0f, static_cast<float>(y) / 10.0f, 0.0f) });
					vertexes.emplace_back(SimpleShader::Vertex{ DirectX::XMFLOAT3( 5.0f, static_cast<float>(y) / 10.0f, 0.0f) });
				}
			}
			for (int x = -50; x <= 50; ++x) {
				if (x % 10 != 0) {
					vertexes.emplace_back(SimpleShader::Vertex{ DirectX::XMFLOAT3(static_cast<float>(x) / 10.0f, -5.0f, 0.0f) });
					vertexes.emplace_back(SimpleShader::Vertex{ DirectX::XMFLOAT3(static_cast<float>(x) / 10.0f,  5.0f, 0.0f) });
				}
			}

			m_GridInner = Object<SimpleShader>(m_d3dDevice, D3D11_PRIMITIVE_TOPOLOGY_LINELIST, vertexes);
			m_GridInner.psColor.color = DirectX::XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f);
		}

		{
			m_Line = Object<SimpleShader>(m_d3dDevice, D3D11_PRIMITIVE_TOPOLOGY_LINELIST, {
				{ DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f) }, { DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f) }
			});
		}

		{
			// TODO: move these logic to separate functions

			const float r = 0.01f;
			const unsigned short resu = 16;
			const unsigned short resv = 16;

			std::vector<SimpleShader::Vertex> vertexes;
			std::vector<unsigned short> indexes;

			for (unsigned short v = 0; v < resv; ++v) {
				const float vv = static_cast<float>(v) / static_cast<float>(resv - 1);

				for (unsigned short u = 0; u <= resu; ++u) {
					const float uu = static_cast<float>(u) / static_cast<float>(resu);

					vertexes.emplace_back(SimpleShader::Vertex{ DirectX::XMFLOAT3(
						r * cosf(2.0f * static_cast<float>(M_PI) * uu) * sinf(static_cast<float>(M_PI) * vv),
						r * sinf(2.0f * static_cast<float>(M_PI) * uu) * sinf(static_cast<float>(M_PI) * vv),
						r * cosf(static_cast<float>(M_PI) * vv)
					) });
				}
			}

			for (unsigned short v = 0; v < resv - 1; ++v) {
				for (unsigned short u = 0; u < resu; ++u) {
					indexes.push_back(v * (resu + 1) + u);
					indexes.push_back((v + 1) * (resu + 1) + u);
				}
			}

			m_Sphere = Object<SimpleShader>(m_d3dDevice, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, vertexes, indexes);
		}
	}

	void WorldView::CreateSizeDependentResources()
	{
		Direct3DPanel::CreateSizeDependentResources();

		// Create depth buffer
		m_DepthStencilView = nullptr;

		CD3D11_TEXTURE2D_DESC depthStencilDesc(
			DXGI_FORMAT_D24_UNORM_S8_UINT,
			m_RenderTargetWidth,
			m_RenderTargetHeight,
			1,
			1,
			D3D11_BIND_DEPTH_STENCIL
		);
		winrt::com_ptr<ID3D11Texture2D> depthStencil;
		winrt::check_hresult(m_d3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, depthStencil.put()));

		CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
		winrt::check_hresult(m_d3dDevice->CreateDepthStencilView(depthStencil.get(), &depthStencilViewDesc, m_DepthStencilView.put()));

		// Setup pipeline
		D3D11_VIEWPORT viewport = { 0 };
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = static_cast<float>(m_RenderTargetWidth);
		viewport.Height = static_cast<float>(m_RenderTargetHeight);
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		m_d3dContext->RSSetViewports(1, &viewport);

		// Recalculate stuff
		DirectX::XMStoreFloat4x4(&m_CameraProjection, DirectX::XMMatrixTranspose(DirectX::XMMatrixPerspectiveFovRH(
			DirectX::XMConvertToRadians(60.0f),
			static_cast<float>(m_RenderTargetWidth) / static_cast<float>(m_RenderTargetHeight),
			0.01f,
			100.0f
		)));

		// Aaand render
		Render();
	}

	void WorldView::Render()
	{
		// In the beginning scanner::Scanner pushes new results, but WorldView is not yet
		// displayed, so no size dependent stuff is created.
		if (m_SwapChain == nullptr)
			return;

		// Calculate some commonly used matrixes
		DirectX::XMFLOAT4X4 identity;
		DirectX::XMStoreFloat4x4(&identity, DirectX::XMMatrixIdentity());

		DirectX::XMFLOAT4X4 view;
		DirectX::XMStoreFloat4x4(&view, DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtRH(
			DirectX::XMVectorSet(
				m_CameraTarget.x + m_CameraDistance * sinf(DirectX::XMConvertToRadians(m_CameraPhi)) * cosf(DirectX::XMConvertToRadians(m_CameraTheta)),
				m_CameraTarget.y + m_CameraDistance * cosf(DirectX::XMConvertToRadians(m_CameraPhi)) * cosf(DirectX::XMConvertToRadians(m_CameraTheta)),
				m_CameraTarget.z + m_CameraDistance * sinf(DirectX::XMConvertToRadians(m_CameraTheta)),
				1.0f
			),
			DirectX::XMVectorSet(m_CameraTarget.x, m_CameraTarget.y, m_CameraTarget.z, 1.0f),
			DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f)
		)));


		// Rendering
		float clearColor[] = { 249.0f / 255.0f, 249.0f / 255.0f, 249.0f / 255.0f, 1.0f };
		m_d3dContext->ClearRenderTargetView(m_RenderTargetView.get(), clearColor);
		m_d3dContext->ClearDepthStencilView(m_DepthStencilView.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		ID3D11RenderTargetView* const targets[] = {
			m_RenderTargetView.get()
		};
		m_d3dContext->OMSetRenderTargets(1, targets, m_DepthStencilView.get());

		m_d3dContext->OMSetDepthStencilState(m_DSSLines.get(), 0);

		m_d3dContext->RSSetState(m_RSSLines.get());


		// Render grid
		m_GridInner.Render(m_d3dContext, m_SimpleShader, view, m_CameraProjection);
		m_GridMeters.Render(m_d3dContext, m_SimpleShader, view, m_CameraProjection);
		m_GridAxisXPos.Render(m_d3dContext, m_SimpleShader, view, m_CameraProjection);
		m_GridAxisXNeg.Render(m_d3dContext, m_SimpleShader, view, m_CameraProjection);
		m_GridAxisYPos.Render(m_d3dContext, m_SimpleShader, view, m_CameraProjection);
		m_GridAxisYNeg.Render(m_d3dContext, m_SimpleShader, view, m_CameraProjection);
		m_GridAxisZ.Render(m_d3dContext, m_SimpleShader, view, m_CameraProjection);

		// Render cameras
		// TODO: do this nicer
		const DirectX::XMFLOAT4 cameraColor(0.0f, 0.3f, 1.0f, 1.0f);
		const float cameraScale = 0.2f;
		//const DirectX::XMFLOAT4 projectionLineColor(1.0f, 0.9f, 0.9f, 1.0f);
		const DirectX::XMFLOAT4 projectionLineColor(1.0f, 0.0f, 0.0f, 0.1f);
		for (const auto& pair : m_Rays) {
			const scanner::Camera* pCamera = pair.first;
			const scanner::IntrinsicParameters& intr = pCamera->getIntrinsicParams();
			const scanner::ExtrinsicParameters& extr = pCamera->getExtrinsicParams();
			const std::optional<scanner::RayInfo>& ray = pair.second;

			const float frLeft = -intr.cx / intr.fx;
			const float frRight = (pCamera->getDevice()->getFrameWidth() - intr.cx) / intr.fx;
			const float frTop = intr.cy / intr.fy;
			const float frBottom = -(pCamera->getDevice()->getFrameHeight() - intr.cy) / intr.fy;

			DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(cameraScale, cameraScale, cameraScale);
			DirectX::XMMATRIX rotation(
				extr.cameraToWorld(0, 0), extr.cameraToWorld(0, 1), extr.cameraToWorld(0, 2), extr.position.x,
				extr.cameraToWorld(1, 0), extr.cameraToWorld(1, 1), extr.cameraToWorld(1, 2), extr.position.y,
				extr.cameraToWorld(2, 0), extr.cameraToWorld(2, 1), extr.cameraToWorld(2, 2), extr.position.z,
				0.0f, 0.0f, 0.0f, 1.0f
			);

			DirectX::XMFLOAT4X4 model;
			DirectX::XMStoreFloat4x4(&model, DirectX::XMMatrixMultiply(rotation, scale));

			DrawLine(DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(frLeft, frTop, 1.0f), cameraColor, model, view);
			DrawLine(DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(frRight, frTop, 1.0f), cameraColor, model, view);
			DrawLine(DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(frLeft, frBottom, 1.0f), cameraColor, model, view);
			DrawLine(DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(frRight, frBottom, 1.0f), cameraColor, model, view);

			DrawLine(DirectX::XMFLOAT3(frLeft, frTop, 1.0f), DirectX::XMFLOAT3(frRight, frTop, 1.0f), cameraColor, model, view);
			DrawLine(DirectX::XMFLOAT3(frRight, frTop, 1.0f), DirectX::XMFLOAT3(frRight, frBottom, 1.0f), cameraColor, model, view);
			DrawLine(DirectX::XMFLOAT3(frRight, frBottom, 1.0f), DirectX::XMFLOAT3(frLeft, frBottom, 1.0f), cameraColor, model, view);
			DrawLine(DirectX::XMFLOAT3(frLeft, frBottom, 1.0f), DirectX::XMFLOAT3(frLeft, frTop, 1.0f), cameraColor, model, view);

			if (ray.has_value()) {
				m_d3dContext->OMSetBlendState(m_BSProjectionLines.get(), nullptr, 0xffffffff);
				DrawLine(
					DirectX::XMFLOAT3(extr.position.x, extr.position.y, extr.position.z),
					DirectX::XMFLOAT3(extr.position.x + ray->t * ray->direction(0), extr.position.y + ray->t * ray->direction(1), extr.position.z + ray->t * ray->direction(2)),
					projectionLineColor,
					view
				);
				m_d3dContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
			}
		}

		m_Sphere.psColor.color = DirectX::XMFLOAT4(0.5f, 0.0f, 1.0f, 1.0f);
		for (const auto& p : m_ScannedPoints) {
			DirectX::XMStoreFloat4x4(&m_Sphere.vsMVP.model, DirectX::XMMatrixTranspose(DirectX::XMMatrixTranslation(p.point3D.x, p.point3D.y, p.point3D.z)));
			m_Sphere.Render(m_d3dContext, m_SimpleShader, view, m_CameraProjection);
		}

		if (m_Point.has_value()) {
			m_Sphere.psColor.color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
			DirectX::XMStoreFloat4x4(&m_Sphere.vsMVP.model, DirectX::XMMatrixTranspose(DirectX::XMMatrixTranslation(m_Point->x, m_Point->y, m_Point->z)));
			m_Sphere.Render(m_d3dContext, m_SimpleShader, view, m_CameraProjection);
		}

		Present();
	}

	void WorldView::UpdateFromTriangulator(const std::optional<cv::Point3f>& point, const std::map<const scanner::Camera*, std::optional<scanner::RayInfo>>& rays)
	{
		m_Point = point;
		m_Rays = rays;

		Render();
	}

	void WorldView::UpdateFromScanner(const std::vector<scanner::ScannedPoint>& scannedPoints)
	{
		m_ScannedPoints = scannedPoints;

		Render();
	}

	void WorldView::OnPointerMoved(Windows::Foundation::IInspectable const& /*sender*/, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e)
	{
		auto p = e.GetCurrentPoint(*this);

		DirectX::XMFLOAT2 currentPos(p.Position().X, p.Position().Y);

		const float dx = currentPos.x - m_LastMoustPos.x;
		const float dy = currentPos.y - m_LastMoustPos.y;

		m_LastMoustPos = currentPos;

		switch (m_CameraManipulation) {
			case CameraManipulation::Orbit:
				m_CameraPhi += 2.0f * dx;
				m_CameraTheta += 2.0f * dy;

				m_CameraTheta = utils::clamp(m_CameraTheta, -89.0f, 89.0f);
				break;
			case CameraManipulation::Pan: {
				// Get the view matrix (or world -> camera)
				// Note: m_CameraTarget is ignored here, because I'm only interested in the
				// rotation part of the transformation.
				DirectX::XMVECTOR eye = DirectX::XMVectorSet(
					m_CameraDistance * sinf(DirectX::XMConvertToRadians(m_CameraPhi)) * cosf(DirectX::XMConvertToRadians(m_CameraTheta)),
					m_CameraDistance * cosf(DirectX::XMConvertToRadians(m_CameraPhi)) * cosf(DirectX::XMConvertToRadians(m_CameraTheta)),
					m_CameraDistance * sinf(DirectX::XMConvertToRadians(m_CameraTheta)),
					1.0f
				);

				DirectX::XMMATRIX view = DirectX::XMMatrixLookAtRH(
					eye,
					DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f),
					DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f)
				);

				// Invert it, because the movement happens in the local coordinate system of the
				// camera. So I want to go camera -> world.
				DirectX::XMMATRIX viewInv = DirectX::XMMatrixInverse(nullptr, view);

				DirectX::XMVECTOR panScreen = DirectX::XMVectorSet(-dx, dy, 0.0f, 0.0f); // Note: w = 0.0f, because I don't want the translation from the view transformation

				DirectX::XMFLOAT3 panWorld;
				DirectX::XMStoreFloat3(&panWorld, DirectX::XMVector4Transform(panScreen, viewInv));

				// Adjust the camera target
				m_CameraTarget.x += 0.01f * panWorld.x;
				m_CameraTarget.y += 0.01f * panWorld.y;
				m_CameraTarget.z += 0.01f * panWorld.z;

				} break;
		}

		if (m_CameraManipulation != CameraManipulation::None)
			Render();
	}

	void WorldView::OnPointerPressed(Windows::Foundation::IInspectable const& /*sender*/, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e)
	{
		auto p = e.GetCurrentPoint(*this);

		m_LastMoustPos.x = p.Position().X;
		m_LastMoustPos.y = p.Position().Y;

		if (p.Properties().PointerUpdateKind() == Microsoft::UI::Input::PointerUpdateKind::LeftButtonPressed) {
			m_CameraManipulation = CameraManipulation::Orbit;
		}
		
		if (p.Properties().PointerUpdateKind() == Microsoft::UI::Input::PointerUpdateKind::MiddleButtonPressed) {
			m_CameraManipulation = CameraManipulation::Pan;
		}
	}

	void WorldView::OnPointerReleased(Windows::Foundation::IInspectable const& /*sender*/, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e)
	{
		auto p = e.GetCurrentPoint(*this);

		m_CameraManipulation = CameraManipulation::None;
	}

	void WorldView::OnPointerWheelChanged(Windows::Foundation::IInspectable const& /*sender*/, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e)
	{
		auto p = e.GetCurrentPoint(*this);

		m_CameraDistance -= 0.002f * static_cast<float>(p.Properties().MouseWheelDelta());

		m_CameraDistance = std::max(m_CameraDistance, 0.25f);

		Render();
	}

	void WorldView::DrawLine(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b, const DirectX::XMFLOAT4& color, const DirectX::XMFLOAT4X4& view)
	{
		DirectX::XMFLOAT4X4 identity;
		DirectX::XMStoreFloat4x4(&identity, DirectX::XMMatrixIdentity());

		DrawLine(a, b, color, identity, view);
	}

	void WorldView::DrawLine(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b, const DirectX::XMFLOAT4& color, const DirectX::XMFLOAT4X4& model, const DirectX::XMFLOAT4X4& view)
	{
		DirectX::XMMATRIX lineMat(
			b.x - a.x, 0.0f, 0.0f, a.x,
			0.0f, b.y - a.y, 0.0f, a.y,
			0.0f, 0.0f, b.z - a.z, a.z,
			0.0f, 0.0f, 0.0f, 1.0f
		);

		DirectX::XMMATRIX modelMat = DirectX::XMLoadFloat4x4(&model);

		DirectX::XMMATRIX trMat = DirectX::XMMatrixMultiply(modelMat, lineMat);
		DirectX::XMFLOAT4X4 tr;
		DirectX::XMStoreFloat4x4(&tr, trMat);

		m_Line.vsMVP.model = tr;
		m_Line.psColor.color = color;

		m_Line.Render(m_d3dContext, m_SimpleShader, view, m_CameraProjection);
	}

}