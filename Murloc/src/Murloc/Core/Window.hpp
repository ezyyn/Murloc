#pragma once
#include "murpch.hpp"

#include "Murloc/Event/Event.hpp"

namespace Murloc {

	struct WindowSpecification
	{
		std::string Title;
		uint32_t Width;
		uint32_t Height;
		bool Decorated;
		bool Fullscreen;
		bool VSync;

		WindowSpecification(const std::string& title = "Murloc App",
							uint32_t width = 1280,
							uint32_t height = 720,
							bool decorated = true,
							bool fullscreen = false,
							bool vsync = true)
			: Title(title), Width(width), Height(height), Decorated(decorated), Fullscreen(fullscreen), VSync(vsync) 
		{}
	};

	class VulkanContext;

	class Window {

	public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() {}

		virtual void ProcessEvents() = 0;

		virtual unsigned int GetWidth() const = 0;
		virtual unsigned int GetHeight() const = 0;

		virtual void SetFullscreen(bool fullscreen) = 0;
		virtual bool FullScreenEnabled() const = 0;

		virtual void SetVSync(bool vsync) = 0;
		virtual void VSyncEnabled(bool vsync) = 0;

		virtual void* GetNativeWindow() const = 0;
		virtual float GetTime() const = 0;

		virtual void SetEventCallback(const EventCallbackFn& fn) = 0;

		static Window* Create(const WindowSpecification& props = WindowSpecification());
	};
}