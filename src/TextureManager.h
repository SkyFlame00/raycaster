#pragma once

#include <memory>
#include <unordered_map>
#include <string>

namespace ray
{
	class Texture;
	class TextureManager;

	using TexturePtr = std::shared_ptr<Texture>;
	using TextureManagerPtr = std::shared_ptr<TextureManager>;

	class Texture
	{
	public:
		Texture(uint8_t* data, uint32_t width, uint32_t height, uint32_t nrChannels);
		~Texture();

		uint32_t GetWidth() const { return m_Width; }
		uint32_t GetHeight() const { return m_Height; }
		uint32_t Sample(float normX, float normY);

	private:
		uint8_t* m_Data;
		uint32_t m_Width;
		uint32_t m_Height;
		uint32_t m_NrChannels;
	};

	class TextureManager
	{
	public:
		static TextureManagerPtr GetInstance();

		~TextureManager();

		TexturePtr LoadTexture(const std::string& path);
		TexturePtr GetTexture(const std::string& path);

	private:
		explicit TextureManager();

		static std::shared_ptr<TextureManager> ms_Instance;
		std::unordered_map<std::string, TexturePtr> m_Textures;
	};
}

