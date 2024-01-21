#pragma once
#include "Effect.h"

namespace dae
{
	class Texture;

	class EffectFire final : public Effect
	{
	public: 
		// CONSTRUCTOR
		EffectFire(ID3D11Device* pDevice, const std::wstring& assetFile);
		virtual ~EffectFire();

		// RULE OF FIVE
		EffectFire(const EffectFire& other) = delete;
		EffectFire& operator=(const EffectFire& other) = delete;
		EffectFire(EffectFire&& other) noexcept = delete;
		EffectFire& operator=(EffectFire&& other) noexcept = delete;

		// MEMBER FUNCTION
		void SetDiffuseMap(Texture* pDiffuseTexture);

	private:
		// MEMBER VARIABLES
		ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable;
	};
}
