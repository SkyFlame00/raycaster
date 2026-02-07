#include "TextureManager.h"

#include <iostream>
#include <stb/stb_image.h>

namespace ray
{
	Texture::Texture(uint8_t* data, uint32_t width, uint32_t height, uint32_t nrChannels)
		: m_Data(data)
		, m_Width(width)
		, m_Height(height)
		, m_NrChannels(nrChannels)
	{
	}

	Texture::~Texture()
	{
		stbi_image_free(m_Data);
	}

	
	uint32_t Texture::Sample(float normX, float normY)
	{
		const uint32_t x = normX * m_Width;
		const uint32_t y = normY * m_Height;
		const uint32_t offset = (y * m_Width + x) * m_NrChannels;
		const uint8_t r = m_Data[offset + 0];
		const uint8_t g = m_Data[offset + 1];
		const uint8_t b = m_Data[offset + 2];
		const uint8_t a = m_Data[offset + 3];
		const uint32_t color = (a << 24) | (r << 16) | (g << 8) | b;

		return color;
	}

	// ===================================================

	std::shared_ptr<TextureManager> TextureManager::ms_Instance = nullptr;

	TextureManagerPtr TextureManager::GetInstance()
	{
		if (ms_Instance == nullptr)
		{
			ms_Instance = std::make_shared<TextureManager>();
		}

		return ms_Instance;
	}

	TextureManager::TextureManager()
	{
	}

	TextureManager::~TextureManager()
	{
	}

	TexturePtr TextureManager::LoadTexture(const std::string& path)
	{
		int32_t width;
		int32_t height;
		int32_t nrChannels;
		uint8_t* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

		if (data == nullptr)
		{
			std::printf("Couldn't load texture at %s", path.c_str());
			return nullptr;
		}

		TexturePtr texture = std::make_shared<Texture>(data, width, height, nrChannels);

		m_Textures[path] = texture;

		return texture;
	}

	TexturePtr TextureManager::GetTexture(const std::string& path)
	{
		auto iter = m_Textures.find(path);

		if (iter == m_Textures.end())
		{
			std::printf("Couldn't fetch texture from cache at %s", path.c_str());
			return nullptr;
		}
		
		return iter->second;
	}
}

