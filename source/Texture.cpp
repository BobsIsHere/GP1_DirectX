#include "pch.h"
#include "Texture.h"
#include <cassert>

using namespace dae;

Texture::Texture(SDL_Surface* pSurface, ID3D11Device* pDevice) :
	//m_pSurface{ pSurface },
	m_pResource{},
	m_pSRV{}
{	
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

	HRESULT hr = pDevice->CreateTexture2D(&desc, &initData, &m_pResource);

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
	SRVDesc.Format = format;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = 1;

	if (m_pResource != nullptr)
	{
		hr = pDevice->CreateShaderResourceView(m_pResource, &SRVDesc, &m_pSRV);
	}
}

Texture::~Texture()
{
	m_pResource->Release();
	m_pSRV->Release();
}

Texture* Texture::LoadTexture(const std::string& path, ID3D11Device* pDevice)
{
	SDL_Surface* pSurface{ IMG_Load(path.data()) };

	assert(pSurface != nullptr);

	Texture* pTexture = new Texture{ pSurface, pDevice };

	SDL_FreeSurface(pSurface);
	return pTexture;  
}

ID3D11ShaderResourceView* Texture::GetShaderResourceView()
{
	return m_pSRV;
}
