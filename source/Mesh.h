#pragma once
#include "Effect.h"

namespace dae
{
	struct Vertex_PosCol
	{
		Vector3 position{};
		Vector2 uv{};
		Vector3 normal{};
		Vector3 tangent{};
	};


	class Mesh final
	{
	public:
		Mesh(ID3D11Device* pDevice, const std::vector<Vertex_PosCol>& vertexData, const std::vector<uint32_t> indexData, 
			 Matrix worldMatrix = Matrix{ Vector4{1, 0, 0, 0}, Vector4{0, 1, 0, 0}, Vector4{0, 0, 1, 0}, Vector4{0, 0, 0, 1} });
		~Mesh();

		//Rule of Five
		Mesh(const Mesh& other) = delete;
		Mesh& operator=(const Mesh& other) = delete;
		Mesh(Mesh&& other) noexcept = delete;
		Mesh& operator=(Mesh&& other) noexcept = delete;

		//Member Functions
		void Render(ID3D11DeviceContext* pDeviceContext, Matrix worldViewProjectionMatrix);
		void SetTextureMaps(Texture* pDiffuseTexture, Texture* pSpecularTexture, Texture* pGlossTexture, Texture* pNormalTexture);
		void SetMatrices(Matrix worldMatrix, Matrix worldViwProj);
		void SetCameraPosition(Vector3 cameraPosition);
		void ToggleSamplerState();
		Matrix GetWorldMatrix() const;

	private:
		//Member Variables
		Effect* m_pEffect;
		ID3DX11EffectTechnique* m_pTechnique;
		ID3D11InputLayout* m_pInputLayout;
		ID3D11Buffer* m_pVertexBuffer;
		ID3D11Buffer* m_pIndexBuffer;

		uint32_t m_NumIndices;
		Matrix m_WorldMatrix;
		Matrix m_WorldViewProjectionMatrix;

		//Member Functions
		void VertexAndInputCreation(ID3D11Device* pDevice, const std::vector<Vertex_PosCol>& vertexData, const std::vector<uint32_t> indexData);
	};
}

