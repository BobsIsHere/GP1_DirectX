#include "pch.h"
#include "EffectVehicle.h"
#include "Texture.h"

using namespace dae;

EffectVehicle::EffectVehicle(ID3D11Device* pDevice, const std::wstring& assetFile) :
	Effect{pDevice, assetFile}
{
	m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
	if (!m_pDiffuseMapVariable->IsValid())
	{
		std::wcout << L"DiffuseMapVariable not valid\n";
	}
	
	m_pSpecularMapVariable = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();
	if (!m_pSpecularMapVariable->IsValid())
	{
		std::wcout << L"SpecularMapVariable not valid\n";
	}

	m_pGlossinessMapVariable = m_pEffect->GetVariableByName("gGlossinessMap")->AsShaderResource();
	if (!m_pGlossinessMapVariable->IsValid())
	{
		std::wcout << L"GlossinessMapVariable not valid\n";
	}

	m_pNormalMapVariable = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();
	if (!m_pNormalMapVariable->IsValid())
	{
		std::wcout << L"NormalMapVariable not valid\n";
	}

	m_pIsNormalMapOn = m_pEffect->GetVariableByName("gUseNormals")->AsScalar();
	if (!m_pIsNormalMapOn->IsValid())
	{
		std::wcout << L"NormalMap bool not valid\n";
	}

	m_pIsNormalMapOn->SetBool(m_IsUsingNormal);
}

EffectVehicle::~EffectVehicle()
{
	m_pDiffuseMapVariable->Release();
	m_pSpecularMapVariable->Release();
	m_pGlossinessMapVariable->Release();
	m_pNormalMapVariable->Release();

	if (m_pIsNormalMapOn != nullptr)
	{
		m_pIsNormalMapOn->Release();
	}
}

void EffectVehicle::SetDiffuseMap(Texture* pDiffuseTexture)
{
	if (m_pDiffuseMapVariable)
	{
		m_pDiffuseMapVariable->SetResource(pDiffuseTexture->GetShaderResourceView());
	}
}

void EffectVehicle::SetSpecularMap(Texture* pSpecularTexture)
{
	if (m_pSpecularMapVariable)
	{
		m_pSpecularMapVariable->SetResource(pSpecularTexture->GetShaderResourceView());
	}
}

void EffectVehicle::SetGlossinessMap(Texture* pGlossTexture)
{
	if (m_pGlossinessMapVariable)
	{
		m_pGlossinessMapVariable->SetResource(pGlossTexture->GetShaderResourceView());
	}
}

void EffectVehicle::SetNormalMap(Texture* pNormalTexture)
{
	if (m_pNormalMapVariable)
	{
		m_pNormalMapVariable->SetResource(pNormalTexture->GetShaderResourceView());
	}
}

void EffectVehicle::ToggleNormalMap()
{
	m_IsUsingNormal = !m_IsUsingNormal;
	m_pIsNormalMapOn->SetBool(m_IsUsingNormal);
}