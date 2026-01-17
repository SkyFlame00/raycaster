#pragma once

#include <memory>
#include <SDL2/SDL.h>

namespace ray
{
	struct SDL_Window;
	struct SDL_Renderer;
	struct SDL_Texture;

	class Window
	{
	public:
		Window(const int32_t width, const int32_t height, ::SDL_Window* sdlWindow, ::SDL_Renderer* sdlRenderer, ::SDL_Texture* sdlTexture);
		~Window();

		uint32_t* GetFramebuffer();
		void Draw();

	private:
		int32_t m_Width;
		int32_t m_Height;
		::SDL_Window* m_SDLWindowPtr;
		::SDL_Renderer* m_SDLRendererPtr;
		::SDL_Texture* m_SDLTexturePtr;
	};
}
