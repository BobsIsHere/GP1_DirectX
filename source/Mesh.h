#pragma once
#include "EffectVehicle.h"

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
		Mesh(ID3D11Device* pDevice, const std::vector<Vertex_PosCol>& vertexData, const std::vector<uint32_t> indexData, Effect* pEffect, 
			 Matrix worldMatrix = Matrix{ Vector4{1, 0, 0, 0}, Vector4{0, 1, 0, 0}, Vector4{0, 0, 1, 0}, Vector4{0, 0, 0, 1} });
		~Mesh();

		//Rule of Five
		Mesh(const Mesh& other) = delete;
		Mesh& operator=(const Mesh& other) = delete;
		Mesh(Mesh&& other) noexcept = delete;
		Mesh& operator=(Mesh&& other) noexcept = delete;

		//Member Functions
		void Render(ID3D11DeviceContext* pDeviceContext, Matrix worldViewProjectionMatrix);
		void SetWorldMatrix(); 
		void SetWVPMatrix(Matrix worldViewProj);
		void SetCameraPosition(Vector3 cameraPosition);
		void ToggleSamplerState();
		void ToggleNormalMap();
		void RotateMesh(float rotation);

		Matrix GetWorldMatrix() const;
		Effect* GetEffect() const { return m_pEffect; };

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
		Matrix m_TranslationMatrix;
		Matrix m_RotationMatrix;
		Matrix m_ScaleMatrix;

		//Member Functions
		void VertexAndInputCreation(ID3D11Device* pDevice, const std::vector<Vertex_PosCol>& vertexData, const std::vector<uint32_t> indexData);
	};
}

