#pragma once
#include "murpch.hpp"

#include "Murloc/Event/Event.hpp"

namespace Murloc {

	struct WindowSpecification
	{
		std::string Title = "Murloc App";
		uint32_t Width = 1280;
		uint32_t Height = 720;
		bool Decorated = true;
		bool Fullscreen = false;
		bool VSync = true;
	};

	class Window {

	public:
		using EventCallbackFn = std::function<bool(Event&)>;

		virtual ~Window() {}

		virtual void OnUpdate() = 0;

		virtual unsigned int GetWidth() const = 0;
		virtual unsigned int GetHeight() const = 0;

		virtual void SetFullscreen(bool fullscreen) = 0;
		virtual bool FullScreenEnabled() const = 0;

		virtual void SetVSync(bool vsync) = 0;
		virtual void VSyncEnabled(bool vsync) = 0;

		virtual void* GetNativeWindow() const = 0;

		virtual void SetEventCallback(const EventCallbackFn& fn) = 0;

		static Window* Create(const WindowSpecification& props = WindowSpecification());
	};
}