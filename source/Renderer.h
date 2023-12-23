#pragma once

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Mesh;
	class Camera;
	class Texture;
	class Effect;

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer& other) = delete;
		Renderer& operator=(const Renderer& other) = delete;
		Renderer(Renderer&& other) noexcept = delete;
		Renderer& operator=(Renderer&& other) noexcept = delete;

		void Update(const Timer* pTimer);
		void Render() const;
		void ToggleSamplerState() const;

	private:
		SDL_Window* m_pWindow{};

		int m_Width{};
		int m_Height{};

		bool m_IsInitialized{ false };

		ID3D11Device* m_pDevice;
		ID3D11DeviceContext* m_pDeviceContext;
		IDXGISwapChain* m_pSwapChain;
		ID3D11Texture2D* m_pDepthStencilBuffer;
		ID3D11DepthStencilView* m_pDepthStencilView;
		ID3D11Resource* m_pRenderTargetBuffer;
		ID3D11RenderTargetView* m_pRenderTargetView;

		Mesh* m_pMesh;
		Camera* m_pCamera;
		Texture* m_pDiffuseTexture;
		Texture* m_pSpecularTexture;
		Texture* m_pGlossinessTexture;
		Texture* m_pNormalTexture;

		//DIRECTX
		HRESULT InitializeDirectX();
	};
}
