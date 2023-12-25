#include "pch.h"
#include "EffectFire.h"
#include "Texture.h"

using namespace dae;

EffectFire::EffectFire(ID3D11Device* pDevice, const std::wstring& assetFile) :
	Effect{pDevice, assetFile}
{
	m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
	if (!m_pDiffuseMapVariable->IsValid())
	{
		std::wcout << L"DiffuseMapVariable not valid\n";
	}
}

EffectFire::~EffectFire()
{
	m_pDiffuseMapVariable->Release();
}

void EffectFire::SetDiffuseMap(Texture* pDiffuseTexture)
{
	if (m_pDiffuseMapVariable)
	{
		m_pDiffuseMapVariable->SetResource(pDiffuseTexture->GetShaderResourceView());
	}
}
