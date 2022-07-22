#pragma once

#include "Pangolin/Core/Window.h"
#include "Pangolin/Renderer/Vulkan/Swapchain.h"

struct GLFWwindow;

namespace PG {

	class Windows_Window : public Window
	{
	public:
		Windows_Window(const WindowSpecification& specification = WindowSpecification());
		~Windows_Window();

		void Init();

		void MapKeyEvents();

		unsigned int GetWidth() const override;
		unsigned int GetHeight() const override;

		void SetFullscreen(bool fullscreen) override;
		bool FullScreenEnabled() const override;
		void SetWindowTitle(const char* title) override;

		void SetVSync(bool vsync) override;
		bool VSyncEnabled() override;

		void* GetNativeWindow() const override;

		float GetTime() const override;

		void AcquireNewSwapchainFrame() override;
		void SwapBuffers() override;

		void ProcessEvents() override;

		void SetEventCallback(const EventCallbackFn& fn) override;

		const void* GetSwapchain() const override { return (void*)m_Swapchain.get(); }


	private:
		struct WindowData {

			bool VSync;
			bool Fullscreen;
			unsigned int Width;
			unsigned int Height;
			std::string Title;

			void* Swapchain;

			EventCallbackFn EventCallback;
		};

		Ref<Swapchain> m_Swapchain;

		WindowData m_Data;

		GLFWwindow* m_NativeWindow;
	};
}