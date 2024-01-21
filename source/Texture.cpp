#include "pch.h"
#include "Texture.h"
#include <cassert>

using namespace dae;

Texture::Texture(SDL_Surface* pSurface, ID3D11Device* pDevice) :
	m_pResource{},
	m_pSRV{},
	m_pSurface{ pSurface },
	m_pSurfacePixels{ (uint32_t*)pSurface->pixels }
{	
	
}

Texture::~Texture()
{	
	m_pResource->Release();
	m_pSRV->Release();

	SDL_FreeSurface(m_pSurface); 
}

ColorRGB Texture::Sample(const Vector2& uv) const
{
	//Sample the correct texel for the given uv
	const float px{ m_pSurface->w * uv.x };
	const float py{ m_pSurface->h * uv.y };
	const uint32_t pIndex{ static_cast<uint32_t>(px) + (static_cast<uint32_t>(py) * m_pSurface->w) };

	uint8_t r{}; 
	uint8_t g{}; 
	uint8_t b{}; 

	SDL_GetRGB(m_pSurfacePixels[pIndex], m_pSurface->format, &r, &g, &b);  

	float rFloat{ r / 255.f }; 
	float gFloat{ g / 255.f }; 
	float bFloat{ b / 255.f }; 

	return ColorRGB{ rFloat,gFloat,bFloat }; 
}

Texture* Texture::LoadTexture(const std::string& path, ID3D11Device* pDevice)
{
	SDL_Surface* pSurface{ IMG_Load(path.data()) };

	assert(pSurface != nullptr);

	Texture* pTexture = new Texture{ pSurface, pDevice };

	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = pSurface->w;
	desc.Height = pSurface->h;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = format;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData{};
	initData.pSysMem = pSurface->pixels;
	initData.SysMemPitch = static_cast<UINT>(pSurface->pitch);
	initData.SysMemSlicePitch = static_cast<UINT>(pSurface->h * pSurface->pitch);

	HRESULT hr = pDevice->CreateTexture2D(&desc, &initData, &pTexture->m_pResource);

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
	SRVDesc.Format = format;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = 1;

	if (pTexture->m_pResource != nullptr)
	{
		hr = pDevice->CreateShaderResourceView(pTexture->m_pResource, &SRVDesc, &pTexture->m_pSRV);
	}
	
	return pTexture;  
}

ID3D11ShaderResourceView* Texture::GetShaderResourceView() const
{
	return m_pSRV;
}
