#pragma once
class Effect final
{
public:
	Effect(ID3D11Device* pDevice, const std::wstring& assetFile);
	~Effect();

	//Rule of Five
	Effect(const Effect&)				 = delete;
	Effect(Effect&&) noexcept			 = delete;
	Effect& operator=(const Effect&)	 = delete;
	Effect& operator=(Effect&&) noexcept = delete;
	
	//Member Functions
	ID3DX11EffectTechnique* GetTechnique() const;

private:
	//Member Functions
	static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);

	//Member Variables
	ID3DX11Effect* m_pEffect;
	ID3DX11EffectTechnique* m_pTechnique;
};

