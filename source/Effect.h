#pragma once
namespace dae
{
	class Texture;

	class Effect final
	{

	public:
		Effect(ID3D11Device* pDevice, const std::wstring& assetFile);
		~Effect();

		//Rule of Five
		Effect(const Effect& other) = delete;
		Effect& operator=(const Effect& other) = delete;
		Effect(Effect&& other) noexcept = delete;
		Effect& operator=(Effect&& other) noexcept = delete;

		//Member Functions
		static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);

		ID3DX11EffectTechnique* GetTechnique() const;
		ID3DX11EffectMatrixVariable* GetMatWorldViewProjVariable() const;

		void SetDiffuseMap(Texture* pDiffuseTexture);

	private:

		//Member Variables
		ID3DX11Effect* m_pEffect;
		ID3DX11EffectTechnique* m_pTechnique;
		ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable;
		ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable;
	};
}