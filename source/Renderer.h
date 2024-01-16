#pragma once

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Mesh;
	class Camera;
	class Texture;
	class EffectVehicle;
	class EffectFire;

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
		void ToggleSamplerState();
		void ToggleShadingModes();
		void ToggleRotation();
		void ToggleNormalMap();
		void ToggleFireMesh();

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

	private:
		SDL_Window* m_pWindow{};

		SamplerStates m_Samples{ SamplerStates::point }; 
		ShadingModes m_ShadingMode{ ShadingModes::combined };

		int m_Width{};
		int m_Height{};

		bool m_IsInitialized{ false };
		bool m_IsRotating{ false };
		bool m_IsShowingNormalMap{ true };
		bool m_IsShowingFireMesh{ true };

		ID3D11Device* m_pDevice;
		ID3D11DeviceContext* m_pDeviceContext;
		IDXGISwapChain* m_pSwapChain;
		ID3D11Texture2D* m_pDepthStencilBuffer;
		ID3D11DepthStencilView* m_pDepthStencilView;
		ID3D11Resource* m_pRenderTargetBuffer;
		ID3D11RenderTargetView* m_pRenderTargetView;

		Mesh* m_pMeshVehicle;
		Mesh* m_pMeshFire;
		Camera* m_pCamera;

		EffectVehicle* m_pEffectVehicle;
		EffectFire* m_pEffectFire;

		Texture* m_pDiffuseTexture;
		Texture* m_pSpecularTexture;
		Texture* m_pGlossinessTexture;
		Texture* m_pNormalTexture;
		Texture* m_pFireTexture;

		//DIRECTX
		HRESULT InitializeDirectX();

		//SOFTWARE
		void RenderMesh_W4();
		void TriangleHandling(int triangleIdx, const Mesh& mesh_transformed);
		void ProcessRenderedTriangle(const Vertex_PosCol& v0, const Vertex_PosCol& v1, const Vertex_PosCol& v2, float w0, float w1, float w2, int px, int py);
		float Remap(float value, float inputMin, float inputMax);
		ColorRGB PixelShading(const Vertex_PosCol& v);
		void VertexTransformationFunction(std::vector<Mesh>& meshes_in) const;
	};
}
