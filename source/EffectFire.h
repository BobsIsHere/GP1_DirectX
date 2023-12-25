#pragma once
#include "Effect.h"

namespace dae
{
	class Texture;

	class EffectFire final : public Effect
	{
	public: 
		EffectFire(ID3D11Device* pDevice, const std::wstring& assetFile);
		virtual ~EffectFire();

		//Rule of Five
		EffectFire(const EffectFire& other) = delete;
		EffectFire& operator=(const EffectFire& other) = delete;
		EffectFire(EffectFire&& other) noexcept = delete;
		EffectFire& operator=(EffectFire&& other) noexcept = delete;

		//Member Functions
		void SetDiffuseMap(Texture* pDiffuseTexture);

	private:
		//Member Variables
		ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable;
		ID3DX11EffectMatrixVariable* m_pMatWorldVariable;
		ID3DX11EffectVectorVariable* m_pCameraPosition;

		ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable;
	};
}
