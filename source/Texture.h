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

		//Software Member Functions
		ColorRGB Sample(const Vector2& uv) const;

		//Hardware Member Functions
		static Texture* LoadTexture(const std::string& path, ID3D11Device* pDevice);
		ID3D11ShaderResourceView* GetShaderResourceView();

	private:
		//Software Member Variables
		SDL_Surface* m_pSurface{ nullptr };
		uint32_t* m_pSurfacePixels{ nullptr };

		//Hardware Member Variables
		ID3D11Texture2D* m_pResource;
		ID3D11ShaderResourceView* m_pSRV;
	};
}