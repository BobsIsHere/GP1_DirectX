#include "pch.h"
#include "Renderer.h"
#include "Mesh.h"
#include "Camera.h"
#include "Texture.h"
#include "EffectVehicle.h"
#include "EffectFire.h"
#include "Utils.h"
#include <algorithm>
#include <execution>

//DirectX headers
#include <dxgi.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <d3dx11effect.h>

namespace dae {

	Renderer::Renderer(SDL_Window* pWindow) :
		m_pWindow(pWindow)
	{
		//Initialize
		SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

		//SOFTWARE
		//Create Buffers
		m_pFrontBuffer = SDL_GetWindowSurface(m_pWindow); 
		m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
		m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels; 

		m_pDepthBufferPixels = new float[m_Width * m_Height]; 

		//Initialize DirectX pipeline
		const HRESULT result = InitializeDirectX();
		if (result == S_OK)
		{
			m_IsInitialized = true;
			std::cout << "DirectX is initialized and ready!\n";
		}
		else
		{
			std::cout << "DirectX initialization failed!\n";
		}

		m_pCamera = new Camera{ {0.f, 0.f, -50.f}, 45.f, 0.1f, 1000.f, float(m_Width) / m_Height }; 

		m_pEffectVehicle = new EffectVehicle{ m_pDevice, L"Resources/DefaultShader.fx" };
		m_pEffectFire = new EffectFire{ m_pDevice, L"Resources/FlatShader.fx" };

		//Load Vehicle Textures
		m_pDiffuseTexture = Texture::LoadTexture("Resources/vehicle_diffuse.png", m_pDevice);
		m_pSpecularTexture = Texture::LoadTexture("Resources/vehicle_specular.png", m_pDevice);
		m_pGlossinessTexture = Texture::LoadTexture("Resources/vehicle_gloss.png", m_pDevice);
		m_pNormalTexture = Texture::LoadTexture("Resources/vehicle_normal.png", m_pDevice);
		m_pFireTexture = Texture::LoadTexture("Resources/fireFX_diffuse.png", m_pDevice);

		std::vector<Vertex_PosCol> vertices{};
		std::vector<uint32_t> indices{};
		const std::string fileNameVehicle{ "Resources/vehicle.obj" };
		const std::string fileNameFire{ "Resources/fireFX.obj" };

		//Vehicle OBJ
		Utils::ParseOBJ(fileNameVehicle, vertices, indices);
		Mesh* pMesh = m_pMeshObjects.emplace_back(new Mesh{ m_pDevice, vertices, indices, m_pEffectVehicle, true });
		m_pEffectVehicle->SetDiffuseMap(m_pDiffuseTexture);
		m_pEffectVehicle->SetSpecularMap(m_pSpecularTexture); 
		m_pEffectVehicle->SetGlossinessMap(m_pGlossinessTexture);
		m_pEffectVehicle->SetNormalMap(m_pNormalTexture); 

		//Fire OBJ
		vertices.clear(); 
		indices.clear(); 

		Utils::ParseOBJ(fileNameFire, vertices, indices);
		pMesh = m_pMeshObjects.emplace_back(new Mesh{ m_pDevice, vertices, indices, m_pEffectFire, false });
		m_pEffectFire->SetDiffuseMap(m_pFireTexture);

		//Togglinng Info
		PrintingInfo(); 
	}

	Renderer::~Renderer()
	{
		delete m_pCamera;
		
		for (auto mesh : m_pMeshObjects)
		{
			delete mesh;
			mesh = nullptr;
		}

		delete m_pEffectVehicle;
		delete m_pDiffuseTexture;
		delete m_pSpecularTexture;
		delete m_pGlossinessTexture;
		delete m_pNormalTexture;
		delete m_pFireTexture;
		delete m_pEffectFire;
		delete[] m_pDepthBufferPixels;

		m_pRenderTargetView->Release(); 
		m_pRenderTargetBuffer->Release(); 
		m_pDepthStencilView->Release(); 
		m_pDepthStencilBuffer->Release(); 
		m_pSwapChain->Release(); 

		if (m_pDeviceContext) 
		{
			m_pDeviceContext->ClearState(); 
			m_pDeviceContext->Flush(); 
			m_pDeviceContext->Release(); 
		}

		m_pDevice->Release();  

		m_pEffectFire = nullptr;
		m_pEffectVehicle = nullptr;
		m_pCamera = nullptr;
		m_pDiffuseTexture = nullptr;
		m_pSpecularTexture = nullptr;
		m_pGlossinessTexture = nullptr;
		m_pNormalTexture = nullptr;
		m_pFireTexture = nullptr;
	}

	void Renderer::Update(const Timer* pTimer)
	{
		m_pCamera->Update(pTimer); 
		
		//variables
		const float yaw{ (pTimer->GetElapsed() * 45.f) * TO_RADIANS }; 
		const Matrix rotation{ Matrix::CreateRotationY(yaw) };

		if (m_IsRotating)
		{
			for (auto mesh : m_pMeshObjects)
			{
				mesh->RotateMesh(yaw);
			}
		}

		for (auto mesh : m_pMeshObjects)
		{
			mesh->SetCameraPosition(m_pCamera->GetCameraOrigin());
		}
	}

	void Renderer::Render() const
	{
		if (!m_IsInitialized)
			return;

		if (m_RasterizerSettings == RasterizerSettings::hardware)
		{
			Render_Hardware();
		}

		if (m_RasterizerSettings == RasterizerSettings::software)
		{
			//@START
			//Lock BackBuffer
			SDL_LockSurface(m_pBackBuffer);
			
			Render_Software();

			//@END
			//Update SDL Surface
			SDL_UnlockSurface(m_pBackBuffer);
			SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0); 
			SDL_UpdateWindowSurface(m_pWindow); 
		}
	}

	// -----------------------------
	//		  HARDWARE PART
	// -----------------------------
	void Renderer::Render_Hardware() const
	{
		//1. CLEAR RTV & DSV
		constexpr float backgroundColor[4] = { 0.39f, 0.59f, 0.93f, 1.f };
		m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, backgroundColor);
		m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

		//2. SET PIPELINE + INVOKE DRAW CALLS (= RENDER)
		const Matrix inverseViewMatrix = m_pCamera->GetInverseViewMatrix(); 
		const Matrix projectionMatrix = m_pCamera->GetProjectionMatrix();

		//first make view projection matrix
		const Matrix worldViewProjectionMatrix{ m_pMeshObjects[0]->GetWorldMatrix() * inverseViewMatrix * projectionMatrix };
		//Render Vehicle Mesh
		m_pMeshObjects[0]->Render(m_pDeviceContext, worldViewProjectionMatrix); 

		if (m_IsShowingFireMesh)
		{
			//first make view projection matrix
			const Matrix worldViewProjectionMatrix{ m_pMeshObjects[1]->GetWorldMatrix() * inverseViewMatrix * projectionMatrix }; 
			//Render Fire Mesh
			m_pMeshObjects[1]->Render(m_pDeviceContext, worldViewProjectionMatrix); 
		}

		//3. PRESENT BACKBUFFER (SWAP)
		m_pSwapChain->Present(0, 0);
	}

	HRESULT Renderer::InitializeDirectX()
	{
		//1. Create Device & Device Context
		//=====
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;
		uint32_t createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
		// |=  -->  bitwise OR operator combined with assignment
		//createDeviceFlags = createDeviceFlags | D3D11_CREATE_DEVICE_DEBUG;
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		HRESULT result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags, &featureLevel, 1, D3D11_SDK_VERSION,
			&m_pDevice, nullptr, &m_pDeviceContext);
		if (FAILED(result))
		{
			return result;
		}

		//Create DXGI Factory
		IDXGIFactory1* pDxgiFactory{};
		//void** --> represents pointer to location that stored another pointer to void
		result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&pDxgiFactory));
		if (FAILED(result))
		{
			return result;
		}

		//2. Create Swapchain
		//=====
		DXGI_SWAP_CHAIN_DESC swapChainDesc{};
		swapChainDesc.BufferDesc.Width = m_Width;
		swapChainDesc.BufferDesc.Height = m_Height;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 1;
		swapChainDesc.Windowed = true;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = 0;

		//Get the handle (HWND) from the SDL backbuffer
		SDL_SysWMinfo sysWMInfo{};
		SDL_GetVersion(&sysWMInfo.version);
		SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);
		swapChainDesc.OutputWindow = sysWMInfo.info.win.window;

		//Create SwapChain
		result = pDxgiFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);
		if (FAILED(result))
		{
			return result;
		}

		//3. Create DepthStencil (DS) and DepthStencilView (DSV)
		//=====
		D3D11_TEXTURE2D_DESC depthStencilDesc{};
		depthStencilDesc.Width = m_Width;
		depthStencilDesc.Height = m_Height;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;

		//View
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
		depthStencilViewDesc.Format = depthStencilDesc.Format;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		result = m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilBuffer);
		if (FAILED(result))
		{
			return result;
		}

		result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
		if (FAILED(result))
		{
			return result;
		}

		//4. Create RenderTarget (RT) & RenderTargetView (RTV)
		//=====

		//Resource
		result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));
		if (FAILED(result))
		{
			return result;
		}

		//View
		result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, nullptr, &m_pRenderTargetView);
		if (FAILED(result))
		{
			return result;
		}

		//5. Bind RTV & DSV to Output Merger Stage
		//=====
		m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

		//6. Set Viewport
		//=====
		D3D11_VIEWPORT viewport{};
		viewport.Width = static_cast<float>(m_Width);
		viewport.Height = static_cast<float>(m_Height);
		viewport.TopLeftX = 0.f;
		viewport.TopLeftY = 0.f;
		viewport.MinDepth = 0.f;
		viewport.MaxDepth = 1.f;
		m_pDeviceContext->RSSetViewports(1, &viewport);

		return result;

		//hidden resource leak of DXGI factory
		pDxgiFactory->Release();
	}

	// -----------------------------
	//		  TOGGLING PART
	// -----------------------------

	// Not const because otherwise i cannnot change the SamplerState
	void Renderer::ToggleSamplerState()
	{
		if (m_RasterizerSettings == RasterizerSettings::hardware)
		{
			for (auto mesh : m_pMeshObjects)
			{
				mesh->ToggleSamplerState();
			}

			switch (m_Samples)
			{
			case SamplerStates::point:
				m_Samples = SamplerStates::linear;
				std::cout << "Sampling Filter: D3D11_FILTER_MIN_MAG_MIP_LINEAR" << std::endl;
				break;

			case SamplerStates::linear:
				m_Samples = SamplerStates::anisotropic;
				std::cout << "Sampling Filter: D3D11_FILTER_MIN_MAG_MIP_ANISOTROPIC" << std::endl;
				break;

			case SamplerStates::anisotropic:
				m_Samples = SamplerStates::point;
				std::cout << "Sampling Filter: D3D11_FILTER_MIN_MAG_MIP_POINT" << std::endl;
				break;
			}
		}
	}

	void Renderer::ToggleShadingModes()
	{
		if (m_RasterizerSettings == RasterizerSettings::software)
		{
			const int amountOfShadingModes{ 4 }; 

			int temp{ static_cast<int>(m_ShadingMode) }; 
			m_ShadingMode = static_cast<ShadingModes>((++temp) % amountOfShadingModes); 

			switch (m_ShadingMode)
			{
			case dae::Renderer::ShadingModes::cosineLambert:
				std::cout << "Render Mode: Cosine Lambert Mode" << std::endl;
				break;
			case dae::Renderer::ShadingModes::diffuseLambert:
				std::cout << "Render Mode: Diffuse Lambert Mode" << std::endl;
				break;
			case dae::Renderer::ShadingModes::specularPhong:
				std::cout << "Render Mode: Specular Phong Mode" << std::endl;
				break;
			case dae::Renderer::ShadingModes::combined:
				std::cout << "Render Mode: Combined Mode" << std::endl;
				break;
			}
		}
	}

	void Renderer::ToggleRotation()
	{
		m_IsRotating = !m_IsRotating;
	}

	void Renderer::ToggleNormalMap()
	{
		for (auto mesh : m_pMeshObjects)
		{
			mesh->ToggleNormalMap(); 
		}

		m_IsNormalMapOn = !m_IsNormalMapOn;

		if (m_IsNormalMapOn)
		{
			std::cout << "Sample normal map: ON\n";
		}
		else
		{
			std::cout << "Sample normal map: OFF\n";
		}
	}

	void Renderer::ToggleFireMesh()
	{
		if (m_RasterizerSettings == RasterizerSettings::hardware)
		{
			m_IsShowingFireMesh = !m_IsShowingFireMesh;
		}
	}

	void Renderer::ToggleRenderingSettings()
	{
		switch (m_RasterizerSettings)
		{
		case Renderer::RasterizerSettings::software:
			m_RasterizerSettings = RasterizerSettings::hardware;
			std::cout << "Render setting: Hardware" << std::endl;
			break;
		case Renderer::RasterizerSettings::hardware:
			m_RasterizerSettings = RasterizerSettings::software;
			std::cout << "Render setting: Software" << std::endl;
			break;
		default:

			break;
		}
	}

	void Renderer::ToggleRenderModes()
	{
		if (m_RasterizerSettings == RasterizerSettings::software)
		{
			const int amountOfRenderModes{ 2 }; 

			int temp{ static_cast<int>(m_RenderMode) }; 
			m_RenderMode = static_cast<RenderMode>((++temp) % amountOfRenderModes); 
		}
	}

	// -----------------------------
	//		  SOFTWARE PART
	// -----------------------------
	void Renderer::Render_Software() const
	{
		//from world to view to projection to screen space
		VertexTransformationFunction(m_pMeshObjects);

		std::fill_n(m_pDepthBufferPixels, m_Width * m_Height, FLT_MAX);

		//clear back buffer
		SDL_FillRect(m_pBackBuffer, &m_pBackBuffer->clip_rect, SDL_MapRGB(m_pBackBuffer->format, 100, 100, 100));

		//for (Mesh* pMesh : m_pMeshObjects) 
		//{
		//	if (pMesh->GetIsHardware())
		//	{
		//		if (pMesh->GetPrimitiveTopology() == PrimitiveTopology::TriangleStrip)
		//		{
		//			const auto& meshIndices = pMesh->GetMeshIndices();
		//			auto& meshVerticesOut = pMesh->GetMeshVerticesOut();
		//			
		//			//extra variable; amount of sides : 0, 1, 2
		//			const auto& maxIdx{ pMesh->GetMeshIndices().size() - 2 };

		//			//go over triangle, per 3 vertices
		//			for (size_t triangleIdx = 0; triangleIdx < meshIndices.size(); triangleIdx += 3) 
		//			{
		//				const Vertex_Out& v0 = meshVerticesOut[meshIndices[triangleIdx + 0]]; 
		//				Vertex_Out& v1 = meshVerticesOut[meshIndices[triangleIdx + 1]];  
		//				Vertex_Out& v2 = meshVerticesOut[meshIndices[triangleIdx + 2]]; 

		//				//if it's odd (oneven)
		//				if (triangleIdx & 1 and pMesh->GetPrimitiveTopology() == PrimitiveTopology::TriangleStrip) 
		//				{
		//					//swap variables, make triangle counter-clockwise
		//					v1 = { meshVerticesOut[meshIndices[triangleIdx + 2]] }; 
		//					v2 = { meshVerticesOut[meshIndices[triangleIdx + 1]] }; 
		//				}

		//				TriangleHandeling(v0, v1, v2, pMesh);
		//			}
		//		}
		//		else if (pMesh->GetPrimitiveTopology() == PrimitiveTopology::TriangleList)
		//		{
		//			const auto& meshIndices = pMesh->GetMeshIndices();
		//			const auto& meshVerticesOut = pMesh->GetMeshVerticesOut();

		//			// Assuming GetMeshIndices() always contains a multiple of 3 indices
		//			for (size_t triangleIdx = 0; triangleIdx < meshIndices.size(); triangleIdx += 3) 
		//			{
		//				const Vertex_Out& v0 = meshVerticesOut[meshIndices[triangleIdx + 0]]; 
		//				const Vertex_Out& v1 = meshVerticesOut[meshIndices[triangleIdx + 1]]; 
		//				const Vertex_Out& v2 = meshVerticesOut[meshIndices[triangleIdx + 2]]; 

		//				//go over triangle, per 3 vertices
		//				TriangleHandeling(v0, v1, v2, pMesh);
		//			}
		//		}
		//	}
		//}

	}

	void Renderer::VertexTransformationFunction(const std::vector<Mesh*>& meshes_in) const
	{
		for (Mesh* pMesh : meshes_in)
		{
			const Matrix worldMatrix{ pMesh->GetWorldMatrix() };
			const Matrix worldViewProjectionMatrix{ worldMatrix * m_pCamera->GetViewMatrix() * m_pCamera->GetProjectionMatrix() }; 

			pMesh->GetMeshVerticesOut().clear();
			auto& meshVerticesOut = pMesh->GetMeshVerticesOut();
			meshVerticesOut.reserve(pMesh->GetMeshVertices().size());

			for (Vertex_PosCol& vertice : pMesh->GetMeshVertices()) 
			{
				Vector4 transformedPosition{ worldViewProjectionMatrix.TransformPoint(Vector4{vertice.position, 1.f}) };

				//first was overriding vertices normal & tangent
				//making temp variable instead 
				const Vector3 newNormal{ worldMatrix.TransformVector(vertice.normal).Normalized() }; 
				const Vector3 newTangent{ worldMatrix.TransformVector(vertice.tangent).Normalized() }; 
				const Vector3 newViewDirection{ worldMatrix.TransformVector(vertice.position) - m_pCamera->GetCameraOrigin() };

				//model to NDC space
				transformedPosition.x /= transformedPosition.w; 
				transformedPosition.y /= transformedPosition.w; 
				transformedPosition.z /= transformedPosition.w; 
				 
				//projection to screen space
				transformedPosition.x = ((transformedPosition.x + 1.f) / 2.f) * m_Width;
				transformedPosition.y = ((1.f - transformedPosition.y) / 2.f) * m_Height;

				Vertex_Out& vertex_out{ pMesh->GetMeshVerticesOut().emplace_back(Vertex_Out{})};
				vertex_out.position = transformedPosition;
				vertex_out.color = vertice.color;
				vertex_out.uv = vertice.uv;
				vertex_out.normal = newNormal;
				vertex_out.tangent = newTangent;
				vertex_out.viewDirection = newViewDirection;
			}
		}
	}

	void Renderer::TriangleHandeling(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, Mesh* mesh_transformed) const
	{
		//frustum culling
		if (v0.position.x < 0 || v0.position.x > m_Width || v0.position.y < 0 || v0.position.y > m_Height ||
			v1.position.x < 0 || v1.position.x > m_Width || v1.position.y < 0 || v1.position.y > m_Height ||
			v2.position.x < 0 || v2.position.x > m_Width || v2.position.y < 0 || v2.position.y > m_Height)
		{
			return;
		}

		//precompute constants
		const Vector2 v2_v1{ v2.position.GetXY() - v1.position.GetXY() };
		const Vector2 v0_v2{ v0.position.GetXY() - v2.position.GetXY() };
		const Vector2 v1_v0{ v1.position.GetXY() - v0.position.GetXY() };

		//scale to increase bounding box size -> no lines between triangles/quads
		const float boundingBoxScale{ 5.f };

		//calculate min & max x of bounding box, clamped to screen
		const float topLeftX{ std::min({v0.position.x, v1.position.x, v2.position.x}) };
		const float topLeftY{ std::min({v0.position.y, v1.position.y, v2.position.y}) };
		const int minX{ Clamp(static_cast<int>(topLeftX - boundingBoxScale), 0, m_Width) };
		const int minY{ Clamp(static_cast<int>(topLeftY - boundingBoxScale), 0, m_Height) };

		//calculate min & max y of bounding box, clamped to screen
		const float bottomRightX{ std::max({v0.position.x, v1.position.x, v2.position.x}) };
		const float bottomRightY{ std::max({v0.position.y, v1.position.y, v2.position.y}) };
		const int maxX{ Clamp(static_cast<int>(bottomRightX + boundingBoxScale), 0, m_Width) };
		const int maxY{ Clamp(static_cast<int>(bottomRightY + boundingBoxScale), 0, m_Height) };

		const Vector2 v0_xy = v0.position.GetXY();
		const Vector2 v1_xy = v1.position.GetXY();
		const Vector2 v2_xy = v2.position.GetXY(); 

		//go over each pixel is in screen space
		for (int px{ minX }; px < maxX; ++px)
		{
			for (int py{ minY }; py < maxY; ++py)
			{
				//define current pixel in screen space
				const Vector2 p{ px + 0.5f, py + 0.5f };

				float w0{ Vector2::Cross(v2_v1, p - v1_xy) };
				float w1{ Vector2::Cross(v0_v2, p - v2_xy) };
				float w2{ Vector2::Cross(v1_v0, p - v0_xy) };

				if (w0 >= 0.f && w1 >= 0.f && w2 >= 0.f)
				{
					ProcessRenderedTriangle(v0, v1, v2, w0, w1, w2, px, py);
				}
			}
		}
	}

	void Renderer::ProcessRenderedTriangle(const Vertex_Out v0, const Vertex_Out v1, const Vertex_Out v2, float w0, float w1, float w2, int px, int py) const
	{
		//variables
		const int bufferIdx{ px + (py * m_Width) };
		ColorRGB finalColour{ 0.f, 0.f, 0.f };

		//using right formula, see slides, has performance gain too
		const float triangleArea{ w0 + w1 + w2 };
		const float invTriangleArea{ 1.f / triangleArea };

		//normalize weights
		w0 *= invTriangleArea;
		w1 *= invTriangleArea;
		w2 *= invTriangleArea;

		//depth buffer -> only for comparing depth values; are not linear
		const float invVerticeZ0{ (1.f / v0.position.z) * w0 };
		const float invVerticeZ1{ (1.f / v1.position.z) * w1 };
		const float invVerticeZ2{ (1.f / v2.position.z) * w2 };
		//used for comparison in depth test and value we store in depth buffer
		float zBufferValue{ 1.f / (invVerticeZ0 + invVerticeZ1 + invVerticeZ2) };

		//check if value is in range of [0,1]
		if (0.f > zBufferValue || zBufferValue > 1.f)
		{
			return;
		}

		if (zBufferValue <= m_pDepthBufferPixels[bufferIdx])
		{
			m_pDepthBufferPixels[bufferIdx] = zBufferValue;

			//intepolate vertex attributes with correct depth
			const float invVerticeW0{ (1.f / v0.position.w) * w0 };
			const float invVerticeW1{ (1.f / v1.position.w) * w1 };
			const float invVerticeW2{ (1.f / v2.position.w) * w2 };
			float wInterpolated{ 1.f / (invVerticeW0 + invVerticeW1 + invVerticeW2) };

			//calculate interpolated uv coordinates
			const Vector2 invUV0{ (v0.uv / v0.position.w) * w0 };
			const Vector2 invUV1{ (v1.uv / v1.position.w) * w1 };
			const Vector2 invUV2{ (v2.uv / v2.position.w) * w2 };
			Vector2 interpolatedUV{ (invUV0 + invUV1 + invUV2) * wInterpolated };

			//clamp interpolated uv value between [0, 1]
			interpolatedUV.x = Clamp(interpolatedUV.x, 0.f, 1.f);
			interpolatedUV.y = Clamp(interpolatedUV.y, 0.f, 1.f);

			//calculate interpolated colour coordinates
			const ColorRGB invColour0{ (v0.color / v0.position.w) * w0 };
			const ColorRGB invColour1{ (v1.color / v1.position.w) * w1 };
			const ColorRGB invColour2{ (v2.color / v2.position.w) * w2 };
			ColorRGB interpolatedColour{ (invColour0 + invColour1 + invColour2) * wInterpolated };

			//calculate interpolated normal coordinates
			const Vector3 invNormal0{ (v0.normal / v0.position.w) * w0 };
			const Vector3 invNormal1{ (v1.normal / v1.position.w) * w1 };
			const Vector3 invNormal2{ (v2.normal / v2.position.w) * w2 };
			Vector3 interpolatedNormal{ (invNormal0 + invNormal1 + invNormal2) * wInterpolated };

			//calculate interpolated tangent coordinates
			const Vector3 invTangent0{ (v0.tangent / v0.position.w) * w0 };
			const Vector3 invTangent1{ (v1.tangent / v1.position.w) * w1 };
			const Vector3 invTangent2{ (v2.tangent / v2.position.w) * w2 };
			Vector3 interpolatedTangent{ (invTangent0 + invTangent1 + invTangent2) * wInterpolated };

			//calculate interpolated viewDirection coordinates
			const Vector3 invViewDirection0{ (v0.viewDirection / v0.position.w) * w0 };
			const Vector3 invViewDirection1{ (v1.viewDirection / v1.position.w) * w1 };
			const Vector3 invViewDirection2{ (v2.viewDirection / v2.position.w) * w2 };
			Vector3 interpolatedViewDirection{ (invViewDirection0 + invViewDirection1 + invViewDirection2) * wInterpolated };

			Vertex_Out vertexOut{};
			vertexOut.uv = interpolatedUV;
			vertexOut.color = interpolatedColour;
			vertexOut.normal = interpolatedNormal.Normalized();
			vertexOut.tangent = interpolatedTangent.Normalized();
			vertexOut.viewDirection = interpolatedViewDirection.Normalized();

			switch (m_RenderMode)
			{
			case RenderMode::finalColour:
				finalColour = PixelShading(vertexOut);
				break;
			case RenderMode::depthBuffer:
				zBufferValue = Remap(zBufferValue, 0.995f, 1.f);
				finalColour = ColorRGB{ zBufferValue, zBufferValue, zBufferValue };
				break;
			}

			finalColour.MaxToOne();

			m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
				static_cast<uint8_t>(finalColour.r * 255),
				static_cast<uint8_t>(finalColour.g * 255),
				static_cast<uint8_t>(finalColour.b * 255));
		}
	}

	float Renderer::Remap(float value, float inputMin, float inputMax) const
	{
		const float temp{ (value - inputMin) / (inputMax - inputMin) }; 
		return temp; 
	}

	ColorRGB Renderer::PixelShading(const Vertex_Out& v) const
	{
		//const variables
		const ColorRGB ambient{ 0.03f, 0.03f , 0.03f };
		const Vector3 lightDirection{ 0.577f, -0.577f, 0.577f };
		const float lightIntensity{ 7.f };
		const float diffuseCoeffient{ 1.f };
		const float shininess{ 25.f };

		//variables
		float observedArea{};
		ColorRGB finalColour{};

		//sample texture maps
		const ColorRGB diffuseColour{ m_pDiffuseTexture->Sample(v.uv) }; 
		const ColorRGB glossColour{ m_pGlossinessTexture->Sample(v.uv) }; 
		const ColorRGB normalTextureSample{ m_pNormalTexture->Sample(v.uv) }; 
		const ColorRGB specularColour{ m_pSpecularTexture->Sample(v.uv) }; 

		//create tangent space transformation matrix
		const Vector3 binormal{ Vector3::Cross(v.normal, v.tangent) };
		const Matrix tangentSpaceAxis{ v.tangent, binormal, v.normal, Vector3{0.f, 0.f, 0.f} };

		// Sample from normal map and multiply it with the matrix
		Vector3 sampledNormal = tangentSpaceAxis.TransformVector((Vector3(normalTextureSample.r, normalTextureSample.g, normalTextureSample.b) * 2.f) - 
																  Vector3{ 1.f, 1.f, 1.f }).Normalized();

		// Calculate observed area
		if (m_IsNormalMapOn)
		{
			//observed area
			observedArea = Vector3::Dot(sampledNormal, -lightDirection);
		}
		else
		{
			//observed area
			observedArea = Vector3::Dot(v.normal, -lightDirection);
		}

		if (observedArea <= 0)
		{
			return ColorRGB{ 0.f, 0.f, 0.f };
		}

		//shading mode calculations
		const ColorRGB exponent{ glossColour * shininess };

		//calculate lambert diffuse
		const ColorRGB lambertDiffuse{ (diffuseCoeffient * diffuseColour) / float(M_PI) };

		//calculate phong reflection
		const Vector3 reflect{ lightDirection - (2.f * Vector3::Dot(sampledNormal, lightDirection) * sampledNormal) };
		const float angle{ std::max(0.f, Vector3::Dot(reflect, -v.viewDirection)) };
		const ColorRGB specular{ specularColour * std::powf(angle, exponent.r) };

		switch (m_ShadingMode)
		{
		case ShadingModes::cosineLambert:  
			finalColour = ColorRGB{ observedArea, observedArea, observedArea }; 
			break;
		case ShadingModes::diffuseLambert:
			finalColour = lightIntensity * lambertDiffuse * observedArea; 
			break;
		case ShadingModes::specularPhong:  
			finalColour = specular * observedArea; 
			break;
		case ShadingModes::combined: 
			finalColour = ((lambertDiffuse * lightIntensity) + specular + ambient) * observedArea; 
			break;
		}

		return finalColour;
	}

	// -----------------------------
	//		PRINTING INFO PART
	// -----------------------------
	void Renderer::PrintingInfo() const
	{
		std::cout << "" << std::endl;
		
		std::cout << RED_COLOR_TEXT << "[KEY BINDINGS - SHARED]" << std::endl;  
		std::cout << "\t [F1] Toggle Rasterizing Settings (HARDWARE/SOFTWARE)" << std::endl; 
		std::cout << "\t [F5] Toggle Rotation (ON/OFF)" << std::endl; 
		std::cout << "\t [F6] Toggle Normal Map (ON/OFF)" << RESET_COLOR_TEXT << std::endl << std::endl; 

		std::cout << BLUE_COLOR_TEXT << "[KEY BINDINGS - HARDWARE]" << std::endl; 
		std::cout << "\t [F4] Cycle Sampler State (POINT/LINEAR/ANISOTROPIC)" << std::endl;
		std::cout << "\t [F7] Toggle FireFX (ON/OFF)" << RESET_COLOR_TEXT << std::endl << std::endl; 

		std::cout << GREEN_COLOR_TEXT << "[KEY BINDINGS - SOFTWARE]" << std::endl;
		std::cout << "\t [F2] Cycle Shading Modes (COMBINED/OBSERVED AREA/DIFFUSE/SPECULAR)" << std::endl;
		std::cout << "\t [F3] Toggle Render Modes (FINAL COLOUR/DEPTH BUFFER)" << RESET_COLOR_TEXT << std::endl << std::endl; 
	}
}	