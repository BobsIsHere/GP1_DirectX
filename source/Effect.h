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

		enum SamplerStates
		{
			point,
			linear,
			anisotropic
		};

		//Member Functions
		static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);

		ID3DX11EffectTechnique* GetTechnique() const;
		ID3DX11EffectMatrixVariable* GetMatWorldViewProjVariable() const;
		ID3D11SamplerState* GetCurrentSamplerState() const;

		void SetDiffuseMap(Texture* pDiffuseTexture);
		void CreateSamplerState(ID3D11Device* pDevice);
		void ToggleSamplerState();
		//void SetSamplerState(SamplerStates filter);

	private:
		//Member Variables
		ID3DX11Effect* m_pEffect;
		ID3DX11EffectTechnique* m_pTechnique;
		ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable;
		ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable;

		ID3DX11EffectSamplerVariable* m_pEffectSamplerVariable;
		ID3D11SamplerState* m_pPointState;
		ID3D11SamplerState* m_pLinearState;
		ID3D11SamplerState* m_pAnisotropicState;
		ID3D11SamplerState* m_pCurrentSamplerState;

		SamplerStates m_SamplerState;
	};
}