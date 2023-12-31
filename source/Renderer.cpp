#include "pch.h"
#include "Renderer.h"
#include "Mesh.h"
#include "Camera.h"
#include "Texture.h"
#include "EffectVehicle.h"
#include "EffectFire.h"
#include "Utils.h"

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

		m_pCamera = new Camera{ 45.f, float(m_Width) / m_Height, {0.f, 0.f, -50.f} };

		m_pEffectVehicle = new EffectVehicle{ m_pDevice, L"Resources/DefaultShader.fx" };
		m_pEffectFire = new EffectFire{ m_pDevice, L"Resources/FlatShader.fx" };

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
		m_pMeshVehicle = new Mesh{ m_pDevice, vertices, indices, m_pEffectVehicle };
		m_pEffectVehicle->SetDiffuseMap(m_pDiffuseTexture);
		m_pEffectVehicle->SetSpecularMap(m_pSpecularTexture);
		m_pEffectVehicle->SetGlossinessMap(m_pGlossinessTexture);
		m_pEffectVehicle->SetNormalMap(m_pNormalTexture);

		//Fire OBJ
		Utils::ParseOBJ(fileNameFire, vertices, indices);
		m_pMeshFire = new Mesh{ m_pDevice, vertices, indices, m_pEffectFire };
		m_pEffectFire->SetDiffuseMap(m_pFireTexture);
	}

	Renderer::~Renderer()
	{
		m_pDevice->Release();
		
		if (m_pDeviceContext)
		{
			m_pDeviceContext->ClearState(); 
			m_pDeviceContext->Flush();
			m_pDeviceContext->Release();
		}

		m_pSwapChain->Release(); 
		m_pDepthStencilBuffer->Release(); 
		m_pDepthStencilView->Release(); 
		m_pRenderTargetBuffer->Release(); 
		m_pRenderTargetView->Release();

		/*delete m_pEffectFire;
		delete m_pEffectVehicle;*/
		delete m_pMeshVehicle;
		delete m_pMeshFire;
		delete m_pCamera;
		delete m_pDiffuseTexture;
		delete m_pSpecularTexture;
		delete m_pGlossinessTexture;
		delete m_pNormalTexture;

		/*m_pEffectFire = nullptr;
		m_pEffectVehicle = nullptr;*/
		m_pMeshVehicle = nullptr;
		m_pMeshFire = nullptr;
		m_pCamera = nullptr;
		m_pDiffuseTexture = nullptr;
		m_pSpecularTexture = nullptr;
		m_pGlossinessTexture = nullptr;
		m_pNormalTexture = nullptr;
	}

	void Renderer::Update(const Timer* pTimer)
	{
		m_pCamera->Update(pTimer);

		m_pMeshVehicle->SetCameraPosition(m_pCamera->GetCameraOrigin());
		m_pMeshFire->SetCameraPosition(m_pCamera->GetCameraOrigin());
	}

	void Renderer::Render() const
	{
		if (!m_IsInitialized)
			return;

		//1. CLEAR RTV & DSV
		constexpr float color[4] = { 0.f, 0.f, 0.3f, 1.f };
		m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, color);
		m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

		//2. SET PIPELINE + INVOKE DRAW CALLS (= RENDER)
		//first make view projection matrix
		const Matrix worldViewProjectionMatrix{ m_pMeshVehicle->GetWorldMatrix() * m_pCamera->GetInverseViewMatrix() * m_pCamera->GetProjectionMatrix() };
		m_pMeshVehicle->Render(m_pDeviceContext, worldViewProjectionMatrix);
		m_pMeshFire->Render(m_pDeviceContext, worldViewProjectionMatrix);

		//3. PRESENT BACKBUFFER (SWAP)
		m_pSwapChain->Present(0, 0);
	}

	void Renderer::ToggleSamplerState() const
	{
		m_pMeshVehicle->ToggleSamplerState();
		m_pMeshFire->ToggleSamplerState();
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
}
