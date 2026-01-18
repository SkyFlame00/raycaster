#include "Platform.h"

#include <SDL2/SDL.h>

#include <memory>
#include <iostream>

#include "./Window.h"

namespace ray
{
	bool Platform::m_IsInitialized = false;

	Platform::Platform()
	{
		Platform::m_IsInitialized = true;
	}

	Platform::~Platform()
	{
		SDL_Quit();
		Platform::m_IsInitialized = false;
	}

	std::shared_ptr<Platform> Platform::CreatePlatform()
	{
		if (m_IsInitialized)
		{
			std::cerr << "Platform was already initialized!\n";
			return nullptr;
		}

		std::cout << "Starting program..." << std::endl;
		if (SDL_Init(SDL_INIT_VIDEO != 0))
		{
			std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
			return nullptr;
		}

		return std::make_shared<Platform>();
	}

	std::shared_ptr<Window> Platform::CreateWindow(const int32_t width, const int32_t height)
	{
		::SDL_Window* sdlWindow = SDL_CreateWindow("Ray casting demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);

		if (!sdlWindow)
		{
			std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
			return nullptr;
		}

		::SDL_Renderer* sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED);
		if (!sdlRenderer)
		{
			std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
			SDL_DestroyWindow(sdlWindow);
			return nullptr;
		}

		::SDL_Texture* sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
		if (!sdlTexture)
		{
			std::cerr << "SDL_CreateTexture Error: " << SDL_GetError() << std::endl;
			SDL_DestroyRenderer(sdlRenderer);
			SDL_DestroyWindow(sdlWindow);
			return nullptr;
		}

		std::shared_ptr<Window> window = std::make_shared<Window>(width, height, sdlWindow, sdlRenderer, sdlTexture);
		m_Windows.push_back(window);

		return window;
	}

	uint64_t Platform::GetElapsedMs()
	{
		return SDL_GetTicks64();
	}

	uint64_t Platform::GetPerformanceCounter()
	{
		return static_cast<uint64_t>(SDL_GetPerformanceCounter());
	}

	uint64_t Platform::GetPerformanceFrequency()
	{
		return static_cast<uint64_t>(SDL_GetPerformanceFrequency());
	}
}

