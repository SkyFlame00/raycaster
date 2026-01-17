#include "Window.h"

namespace ray
{
	Window::Window(const int32_t width, const int32_t height, ::SDL_Window* sdlWindow, ::SDL_Renderer* sdlRenderer, ::SDL_Texture* sdlTexture)
		: m_Width(width)
		, m_Height(height)
		, m_SDLWindowPtr(sdlWindow)
		, m_SDLRendererPtr(sdlRenderer)
		, m_SDLTexturePtr(sdlTexture)
	{
	}

	Window::~Window()
	{
		SDL_DestroyTexture(m_SDLTexturePtr);
		SDL_DestroyRenderer(m_SDLRendererPtr);
		SDL_DestroyWindow(m_SDLWindowPtr);
	}

	uint32_t* Window::GetFramebuffer()
	{
		void* pixels;
		int pitch; // it's measured in bytes
		SDL_LockTexture(m_SDLTexturePtr, nullptr, &pixels, &pitch);

		return static_cast<uint32_t*>(pixels);
	}

	void Window::Draw()
	{
		SDL_UnlockTexture(m_SDLTexturePtr);
		SDL_RenderClear(m_SDLRendererPtr);
		SDL_RenderCopy(m_SDLRendererPtr, m_SDLTexturePtr, nullptr, nullptr);
		SDL_RenderPresent(m_SDLRendererPtr);
	}
}

