#include "pch.h"
#include "Mesh.h" 
#include <cassert>

dae::Mesh::Mesh(ID3D11Device* pDevice, const std::vector<Vertex_PosCol>& vertexData, const std::vector<uint32_t> indexData, Effect* pEffect, Matrix worldMatrix) :
	m_NumIndices{},
	m_pIndexBuffer{},
	m_pInputLayout{},
	m_WorldMatrix{ worldMatrix },
	m_pEffect{ pEffect } 
{
	m_pTechnique = m_pEffect->GetTechnique();

	// Vertex / Input Layout and Buffer
	VertexAndInputCreation(pDevice, vertexData, indexData);
}

dae::Mesh::~Mesh()
{
	m_pVertexBuffer->Release();
	m_pIndexBuffer->Release();
	m_pInputLayout->Release();

	// Double deletion, effect and technique does not get handeled in Mesh & gets deleted in Effect class.
	// Effect object is now owned by mesh, simply utilised. Technique is derrived from Effect.
	//m_pTechnique->Release();

	/*if (m_pEffect)
	{
		delete m_pEffect;
		m_pEffect = nullptr;
	}*/
}

void dae::Mesh::Render(ID3D11DeviceContext* pDeviceContext, Matrix worldViewProjectionMatrix)
{
	//1. Set Primitive Topology
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//2. Set Input Layout
	pDeviceContext->IASetInputLayout(m_pInputLayout);

	//3. Set Vertex Buffer
	constexpr UINT stride = sizeof(Vertex_PosCol);
	constexpr UINT offset = 0;
	pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
	SetWorldMatrix(); 
	SetWVPMatrix(worldViewProjectionMatrix);

	//4. Set Index Buffer
	pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//5. Draw
	D3DX11_TECHNIQUE_DESC techDesc{};
	m_pTechnique->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_pTechnique->GetPassByIndex(p)->Apply(0, pDeviceContext);
		pDeviceContext->DrawIndexed(m_NumIndices, 0, 0);
	}
}

void dae::Mesh::SetWorldMatrix()
{
	m_WorldMatrix = m_ScaleMatrix * m_RotationMatrix * m_TranslationMatrix;
	
	m_pEffect->SetWorldMatrix(m_WorldMatrix);
}

void dae::Mesh::SetWVPMatrix(Matrix worldViewProj)
{
	m_pEffect->SetWorldViewProjectionMatrix(worldViewProj);
}

void dae::Mesh::SetCameraPosition(Vector3 cameraPosition)
{
	m_pEffect->SetCameraPosition(cameraPosition);
}

dae::Matrix dae::Mesh::GetWorldMatrix() const
{
	return m_WorldMatrix;
}

void dae::Mesh::ToggleSamplerState()
{
	m_pEffect->ToggleSamplerState();
}

void dae::Mesh::ToggleNormalMap()
{
	m_pEffect->ToggleNormalMap();
}

void dae::Mesh::RotateMesh(float rotation)
{
	m_RotationMatrix = Matrix::CreateRotationY(rotation) * m_RotationMatrix;
}

void dae::Mesh::VertexAndInputCreation(ID3D11Device* pDevice, const std::vector<Vertex_PosCol>& vertexData, const std::vector<uint32_t> indexData)
{
	//Create Vertex Layout
	static constexpr uint32_t numElements{ 4 };
	D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

	// Position
	vertexDesc[0].SemanticName = "POSITION";
	vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[0].AlignedByteOffset = 0;
	vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	//UV
	vertexDesc[1].SemanticName = "TEXCOORD";
	vertexDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	vertexDesc[1].AlignedByteOffset = 12;
	vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	//Normal
	vertexDesc[2].SemanticName = "NORMAL";
	vertexDesc[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[2].AlignedByteOffset = 20;
	vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	//Tangent
	vertexDesc[3].SemanticName = "TANGENT";
	vertexDesc[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[3].AlignedByteOffset = 32;
	vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	//Create Input Layout
	D3DX11_PASS_DESC passDesc{};
	m_pTechnique->GetPassByIndex(0)->GetDesc(&passDesc);

	HRESULT result = pDevice->CreateInputLayout(
		vertexDesc,
		numElements,
		passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize,
		&m_pInputLayout);

	if (FAILED(result))
	{
		assert(false);
	}

	//Create Vertex Buffer
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(Vertex_PosCol) * static_cast<uint32_t>(vertexData.size());
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = vertexData.data();

	result = pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer);
	if (FAILED(result))
	{
		return;
	}

	//Create Index Buffer
	m_NumIndices = static_cast<uint32_t>(indexData.size());
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(uint32_t) * m_NumIndices;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	initData.pSysMem = indexData.data();

	result = pDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);
	if (FAILED(result))
	{
		return;
	}
}
