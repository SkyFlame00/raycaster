#pragma once

#include <memory>
#include <vector>

namespace ray
{
	class Window;

	class Platform
	{
	public:
		Platform();
		~Platform();

		static std::shared_ptr<Platform> CreatePlatform();
		std::shared_ptr<Window> CreateWindow(const int32_t width, const int32_t height);
		uint64_t GetElapsedMs();

	private:
		static bool m_IsInitialized;
		std::vector<std::shared_ptr<Window>> m_Windows;
	};
}

