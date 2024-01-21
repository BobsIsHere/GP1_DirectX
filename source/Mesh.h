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

		ColorRGB color{};
	};

	struct Vertex_Out
	{
		Vector4 position{};
		ColorRGB color{};
		Vector2 uv{};
		Vector3 normal{};
		Vector3 tangent{};
		Vector3 viewDirection{};
	};

	enum class PrimitiveTopology
	{
		TriangleList,
		TriangleStrip
	};

	class Mesh final
	{
	public:
		// CONSTRUCTOR AND DESTRUCTOR
		Mesh(ID3D11Device* pDevice, const std::vector<Vertex_PosCol>& vertexData, const std::vector<uint32_t> indexData, Effect* pEffect, bool isSoftware,
			 Matrix worldMatrix = Matrix{ Vector4{1, 0, 0, 0}, Vector4{0, 1, 0, 0}, Vector4{0, 0, 1, 0}, Vector4{0, 0, 0, 1} });
		~Mesh();

		// RULE OF FIVE
		Mesh(const Mesh& other) = delete;
		Mesh& operator=(const Mesh& other) = delete;
		Mesh(Mesh&& other) noexcept = delete;
		Mesh& operator=(Mesh&& other) noexcept = delete;

		// HARDWARE MEMBER FUNCTIONS
		void Render(ID3D11DeviceContext* pDeviceContext, Matrix worldViewProjectionMatrix);
		void SetWorldMatrix(); 
		void SetWVPMatrix(Matrix worldViewProj);
		void SetCameraPosition(Vector3 cameraPosition);
		void ToggleSamplerState();
		void ToggleNormalMap();
		void RotateMesh(float rotation);

		Matrix GetWorldMatrix() const;
		Effect* GetEffect() const;
		bool GetIsInSoftwareMode() const;

		// SOFTWARE MEMBER FUNCTIONS
		std::vector<Vertex_PosCol> GetMeshVertices() const;
		std::vector<uint32_t> GetMeshIndices() const; 
		PrimitiveTopology GetPrimitiveTopology() const;
		std::vector<Vertex_Out>& GetMeshVerticesOut(); 

	private:
		// HARDWARE MEMBER VARIABLES
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

		// SOFTWARE MEMBER VARIABLES
		std::vector<Vertex_PosCol> m_Vertices{};
		std::vector<uint32_t> m_Indices{};
		PrimitiveTopology m_PrimitiveTopology{ PrimitiveTopology::TriangleList }; 
		std::vector<Vertex_Out> m_VerticesOut{};

		bool m_IsInSoftwareMode{ true }; 

		// MEMBER FUNCTION
		void VertexAndInputCreation(ID3D11Device* pDevice, const std::vector<Vertex_PosCol>& vertexData, const std::vector<uint32_t> indexData);
	};
}

