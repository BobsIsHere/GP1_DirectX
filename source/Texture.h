#pragma once

namespace dae
{
	class Texture
	{
	public:
		Texture(SDL_Surface* pSurface, ID3D11Device* pDevice);
		~Texture();

		//Rule of Five
		Texture(const Texture& other) = delete;
		Texture& operator=(const Texture& other) = delete;
		Texture(Texture&& other) noexcept = delete;
		Texture& operator=(Texture&& other) noexcept = delete;

		//Member Functions
		static Texture* LoadTexture(const std::string& path, ID3D11Device* pDevice);
		ID3D11ShaderResourceView* GetShaderResourceView();

	private:
		SDL_Surface* m_pSurface;
		ID3D11Texture2D* m_pResource;
		ID3D11ShaderResourceView* m_pSRV;
	};
}