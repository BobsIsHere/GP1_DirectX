#pragma once
#include "Effect.h"

namespace dae
{
	class Texture;

	class EffectVehicle final : public Effect
	{

	public:
		EffectVehicle(ID3D11Device* pDevice, const std::wstring& assetFile);
		~EffectVehicle();

		//Rule of Five
		EffectVehicle(const EffectVehicle& other) = delete;
		EffectVehicle& operator=(const EffectVehicle& other) = delete;
		EffectVehicle(EffectVehicle&& other) noexcept = delete;
		EffectVehicle& operator=(EffectVehicle&& other) noexcept = delete;

		void SetDiffuseMap(Texture* pDiffuseTexture);
		void SetSpecularMap(Texture* pSpecularTexture);
		void SetGlossinessMap(Texture* pGlossTexture);
		void SetNormalMap(Texture* pNormalTexture);

	private:
		//Member Variables
		ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable;
		ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable;
		ID3DX11EffectShaderResourceVariable* m_pGlossinessMapVariable; 
		ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable;
	};
}