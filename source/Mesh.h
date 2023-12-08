#pragma once
#include "Effect.h"

namespace dae
{
	struct Vertex_PosCol
	{
		Vector3 position{};
		ColorRGB color{};
	};


	class Mesh final
	{
	public:
		Mesh(ID3D11Device* pDevice, const std::vector<Vertex_PosCol>& vertexData, const std::vector<uint32_t> indexData);
		~Mesh();

		//Rule of Five
		Mesh(const Mesh&)				 = delete;
		Mesh(Mesh&&) noexcept			 = delete;
		Mesh& operator=(const Mesh&)	 = delete;
		Mesh& operator=(Mesh&&) noexcept = delete;

		//Member Functions
		void Render(ID3D11DeviceContext* pDeviceContext);

	private:
		//Member variables
		Effect* m_pEffect;
		ID3DX11EffectTechnique* m_pTechnique;
		ID3D11InputLayout* m_pInputLayout;
		ID3D11Buffer* m_pVertexBuffer;
		ID3D11Buffer* m_pIndexBuffer;

		uint32_t m_NumIndices;
	};
}

