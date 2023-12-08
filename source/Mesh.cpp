#include "pch.h"
#include "Mesh.h" 
#include <cassert>

dae::Mesh::Mesh(ID3D11Device* pDevice, const std::vector<Vertex_PosCol>& vertexData, const std::vector<uint32_t> indexData) :
	m_NumIndices{},
	m_pIndexBuffer{},
	m_pInputLayout{}
{
	const std::wstring& assetName{ L"Resources/PosCol3D.fx" };
	m_pEffect = new Effect{ pDevice, assetName };
	m_pTechnique = m_pEffect->GetTechnique();

	//Create Vertex Layout
	static constexpr uint32_t numElements{ 2 };
	D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

	vertexDesc[0].SemanticName = "POSITION";
	vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[0].AlignedByteOffset = 0;
	vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[1].SemanticName = "COLOR";
	vertexDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[1].AlignedByteOffset = 12;
	vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

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

dae::Mesh::~Mesh()
{
	delete m_pEffect;
	m_pEffect = nullptr;

	m_pTechnique->Release();
	m_pInputLayout->Release();
	m_pVertexBuffer->Release();
	m_pIndexBuffer->Release();
}

void dae::Mesh::Render(ID3D11DeviceContext* pDeviceContext)
{
	//1. Set Primitive Topology
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//2. Set Input Layout
	pDeviceContext->IASetInputLayout(m_pInputLayout);

	//3. Set Vertex Buffer
	constexpr UINT stride = sizeof(Vertex_PosCol);
	constexpr UINT offset = 0;
	pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

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
