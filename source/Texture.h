#pragma once

namespace dae
{
	class Texture
	{
	public:
		// CONSTRUCTOR AND DESTRUCTOR
		Texture(SDL_Surface* pSurface, ID3D11Device* pDevice);
		~Texture();

		// RULE OF FIVE
		Texture(const Texture& other) = delete;
		Texture& operator=(const Texture& other) = delete;
		Texture(Texture&& other) noexcept = delete;
		Texture& operator=(Texture&& other) noexcept = delete;

		// SOFTWARE MEMBER FUNCTION
		ColorRGB Sample(const Vector2& uv) const;

		// HARDWARE MEMBER FUNCTIONS
		static Texture* LoadTexture(const std::string& path, ID3D11Device* pDevice);
		ID3D11ShaderResourceView* GetShaderResourceView() const;

	private:
		// SOFTWARE MEMBER VARIABLES
		SDL_Surface* m_pSurface{ nullptr };
		uint32_t* m_pSurfacePixels{ nullptr };

		// HARDWARE MEMBER VARIABLES
		ID3D11Texture2D* m_pResource;
		ID3D11ShaderResourceView* m_pSRV;
	};
}