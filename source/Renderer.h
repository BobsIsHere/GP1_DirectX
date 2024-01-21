#pragma once

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	struct Vertex_Out;

	class Mesh;
	class Camera;
	class Texture;
	class EffectVehicle;
	class EffectFire;

	class Renderer final
	{
	public:
		// CONSTRUCTOR AND DESTRUCTOR
		Renderer(SDL_Window* pWindow);
		~Renderer();

		// RULE OF FIVE
		Renderer(const Renderer& other) = delete;
		Renderer& operator=(const Renderer& other) = delete;
		Renderer(Renderer&& other) noexcept = delete;
		Renderer& operator=(Renderer&& other) noexcept = delete;

		// ENUMS
		enum class RasterizerSettings
		{
			software,
			hardware
		};

		enum class RenderMode
		{
			finalColour,
			depthBuffer 
		};

		enum class SamplerStates
		{
			point,
			linear,
			anisotropic
		};

		enum class ShadingModes
		{
			cosineLambert,
			diffuseLambert,
			specularPhong,
			combined
		};

		// MEMBER FUNCTIONS
		void Update(const Timer* pTimer);
		void Render() const;
		void ToggleSamplerState();
		void ToggleShadingModes();
		void ToggleRotation();
		void ToggleNormalMap();
		void ToggleFireMesh();
		void ToggleRenderingSettings();
		void ToggleRenderModes();

	private:
		// SHARED VARIABLES
		SDL_Window* m_pWindow{};

		Texture* m_pDiffuseTexture; 
		Texture* m_pSpecularTexture; 
		Texture* m_pGlossinessTexture; 
		Texture* m_pNormalTexture; 

		SamplerStates m_Samples{ SamplerStates::point }; 
		ShadingModes m_ShadingMode{ ShadingModes::combined };
		RasterizerSettings m_RasterizerSettings{ RasterizerSettings::hardware };

		bool m_IsRotating{ false };
		bool m_IsNormalMapOn{ true };

		// HARDWARE VARIABLES
		int m_Width{};
		int m_Height{};

		bool m_IsInitialized{ false };
		bool m_IsShowingFireMesh{ true };

		ID3D11Device* m_pDevice;
		ID3D11DeviceContext* m_pDeviceContext;
		IDXGISwapChain* m_pSwapChain;
		ID3D11Texture2D* m_pDepthStencilBuffer;
		ID3D11DepthStencilView* m_pDepthStencilView;
		ID3D11Resource* m_pRenderTargetBuffer;
		ID3D11RenderTargetView* m_pRenderTargetView;

		std::vector<Mesh*> m_pMeshObjects;
		Camera* m_pCamera;

		EffectVehicle* m_pEffectVehicle;
		EffectFire* m_pEffectFire;

		Texture* m_pFireTexture;

		// SOFTWARE VARIABLES
		RenderMode m_RenderMode{ RenderMode::finalColour };

		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};

		float* m_pDepthBufferPixels{};

		// DIRECTX FUNCTIONS
		void Render_Hardware() const;

		HRESULT InitializeDirectX();

		// SOFTWARE FUNCTIONS
		void Render_Software() const;
		void VertexTransformationFunction(const std::vector<Mesh*>& meshes_in) const;
		void TriangleHandeling(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, Mesh* mesh_transformed) const;
		void ProcessRenderedTriangle(const Vertex_Out v0, const Vertex_Out v1, const Vertex_Out v2, float w0, float w1, float w2, int px, int py) const;

		float Remap(float value, float inputMin, float inputMax) const;
		ColorRGB PixelShading(const Vertex_Out& v) const;

		// MEMBER FUCTIONS
		void PrintingInfo() const;
	};
}
