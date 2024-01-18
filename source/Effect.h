#pragma once
namespace dae
{
	class Effect
	{
	public:
		Effect(ID3D11Device* pDevice, const std::wstring& assetFile);
		virtual ~Effect();

		//Rule of Five
		Effect(const Effect& other) = delete;
		Effect& operator=(const Effect& other) = delete;
		Effect(Effect&& other) noexcept = delete;
		Effect& operator=(Effect&& other) noexcept = delete;

		enum class SamplerStates
		{
			point,
			linear,
			anisotropic
		};

		//Member Functions
		virtual ID3DX11EffectTechnique* GetTechnique() const;
		virtual ID3DX11EffectMatrixVariable* GetMatWorldViewProjVariable() const;
		virtual ID3D11SamplerState* GetCurrentSamplerState() const;

		virtual void SetWorldViewProjectionMatrix(Matrix worldViewProjectionMatrix);
		virtual void SetWorldMatrix(Matrix worldMatrix);
		virtual void SetCameraPosition(Vector3 cameraPosition);
		virtual void CreateSamplerState(ID3D11Device* pDevice);
		virtual void ToggleSamplerState();

	protected:
		//Member Functions
		virtual ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);

		//Member Variables
		ID3DX11Effect* m_pEffect;
		ID3DX11EffectTechnique* m_pTechnique;

		ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable;
		ID3DX11EffectMatrixVariable* m_pMatWorldVariable;
		ID3DX11EffectVectorVariable* m_pCameraPosition;

		ID3DX11EffectSamplerVariable* m_pEffectSamplerVariable;
		ID3D11SamplerState* m_pPointState;
		ID3D11SamplerState* m_pLinearState;
		ID3D11SamplerState* m_pAnisotropicState;
		ID3D11SamplerState* m_pCurrentSamplerState;

		SamplerStates m_SamplerState;

		bool m_IsUsingNormal{ true };
	};
}