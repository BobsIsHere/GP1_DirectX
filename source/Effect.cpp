#include "pch.h"
#include "Effect.h"
#include "Texture.h"

using namespace dae;

Effect::Effect(ID3D11Device* pDevice, const std::wstring& assetFile)
{
	m_pEffect = LoadEffect(pDevice, assetFile); 
	m_SamplerState = SamplerStates::point;

	m_pTechnique = m_pEffect->GetTechniqueByName("DefaultTechnique");
	if (!m_pTechnique->IsValid())
	{
		std::wcout << L"Technique not valid\n";
	}

	m_pMatWorldViewProjVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
	if (!m_pMatWorldViewProjVariable->IsValid())
	{
		std::wcout << L"WorldViewProjection matrix not valid\n";
	}

	m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
	if (!m_pDiffuseMapVariable->IsValid())
	{
		std::wcout << L"DiffuseMapVariable not valid\n";
	}

	m_pEffectSamplerVariable = m_pEffect->GetVariableByName("gSamplerState")->AsSampler();
	if (!m_pEffectSamplerVariable->IsValid())
	{
		std::wcout << L"SamplerVariable not valid\n";
	}
}

Effect::~Effect()
{
	m_pPointState->Release(); 
	m_pLinearState->Release(); 
	m_pAnisotropicState->Release();
	
	m_pMatWorldViewProjVariable->Release();
	m_pDiffuseMapVariable->Release();
	m_pTechnique->Release();
	m_pEffect->Release();
}

ID3DX11EffectTechnique* Effect::GetTechnique() const
{
	return m_pTechnique;
}

ID3DX11EffectMatrixVariable* Effect::GetMatWorldViewProjVariable() const
{
	return m_pMatWorldViewProjVariable;
}

void Effect::SetDiffuseMap(Texture* pDiffuseTexture)
{
	if (m_pDiffuseMapVariable)
	{
		m_pDiffuseMapVariable->SetResource(pDiffuseTexture->GetShaderResourceView());
	}
}

void Effect::CreateSamplerState(ID3D11Device* pDevice)
{
	HRESULT result{};
	D3D11_SAMPLER_DESC samplerDesc{};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;  
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;  
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;  
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER; 
	samplerDesc.MinLOD = 0;  
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;  

	switch (m_SamplerState)
	{
	case point:
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		result = pDevice->CreateSamplerState(&samplerDesc, &m_pPointState);

		if (FAILED(result))
		{
			std::wcout << L"Creation point state failed";
		}
		break;
	case linear: 
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; 
		result = pDevice->CreateSamplerState(&samplerDesc, &m_pLinearState);

		if (FAILED(result)) 
		{
			std::wcout << L"Creation linear state failed";
		}
		break;
	case anisotropic: 
		samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC; 
		result = pDevice->CreateSamplerState(&samplerDesc, &m_pAnisotropicState);

		if (FAILED(result)) 
		{
			std::wcout << L"Creation anisotropic state failed";
		}
		break;
	}

	if (!m_pCurrentSamplerState)
	{
		m_pCurrentSamplerState = m_pPointState;
	}
}

//void Effect::SetSamplerState(SamplerStates filter)
//{
//	switch (filter)
//	{
//	case point: 
//		m_pCurrentSamplerState = m_pPointState;
//		break;
//	case linear:
//		m_pCurrentSamplerState = m_pLinearState;
//		break;
//	case anisotropic: 
//		m_pCurrentSamplerState = m_pAnisotropicState;
//		break;
//	}
//}

void Effect::ToggleSamplerState()
{
	m_pEffectSamplerVariable->SetSampler(0, m_pCurrentSamplerState);

	// Toggle between sampler states (Point -> Linear -> Anisotropic -> Point)
	if (m_pCurrentSamplerState == m_pPointState)
	{
		m_pCurrentSamplerState = m_pLinearState;
	}
	else if (m_pCurrentSamplerState == m_pLinearState)
	{
		m_pCurrentSamplerState = m_pAnisotropicState;
	}
	else if (m_pCurrentSamplerState == m_pAnisotropicState) 
	{
		m_pCurrentSamplerState = m_pPointState; 
	}
}

ID3D11SamplerState* Effect::GetCurrentSamplerState() const
{
	return m_pCurrentSamplerState;
}

ID3DX11Effect* Effect::LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
{
	HRESULT result{};
	ID3D10Blob* pErrorBlob{ nullptr };
	ID3DX11Effect* pEffect{};

	DWORD shaderFlags = 0;
#if defined( DEBUG ) || defined( _DEBUG )
	shaderFlags |= D3DCOMPILE_DEBUG; 
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
	
	result = D3DX11CompileEffectFromFile(assetFile.c_str(),
										 nullptr,
										 nullptr,
										shaderFlags,
										 0,
										 pDevice,
										&pEffect,
										&pErrorBlob);

	if (FAILED(result))
	{
		if (pErrorBlob != nullptr)
		{
			const char* pErrors = static_cast<char*>(pErrorBlob->GetBufferPointer());

			std::wstringstream ss;
			for (unsigned int i = 0; i < pErrorBlob->GetBufferSize(); ++i)
			{
				ss << pErrors[i];
			}

			OutputDebugStringW(ss.str().c_str());
			pErrorBlob->Release();
			pErrorBlob = nullptr;

			std::wcout << ss.str() << std::endl;
		}
		else
		{
			std::wstringstream ss;
			ss << "EffectLoader: Failed to CreateEffectFromFile!\nPath: " << assetFile;
			std::wcout << ss.str() << std::endl;
			return nullptr;
		}
	}

	return pEffect; 
}
