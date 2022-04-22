#pragma once

#include "Murloc/Core/Window.hpp"

struct GLFWwindow;

namespace Murloc {

	class Windows_Window : public Window
	{
	public:
		Windows_Window(const WindowSpecification& specification = WindowSpecification());

		~Windows_Window();
		void Init();

		unsigned int GetWidth() const override;
		unsigned int GetHeight() const override;

		void SetFullscreen(bool fullscreen) override;
		bool FullScreenEnabled() const override;

		void SetVSync(bool vsync) override;
		void VSyncEnabled(bool vsync) override;

		void* GetNativeWindow() const override;

		float GetTime() const override;

		void OnUpdate() override;

		void SetEventCallback(const EventCallbackFn& fn) override;

		VulkanContext* GetContext() const override { return m_Context; };

	private:

		struct WindowData {

			bool VSync;
			bool Fullscreen;
			unsigned int Width;
			unsigned int Height;
			std::string Title;

			EventCallbackFn EventCallback;
		};

		WindowData m_Data;

		VulkanContext* m_Context;

		GLFWwindow* m_NativeWindow;
	};
}