#include "murpch.hpp"

#include "Windows_Window.hpp"

#include "Murloc/Event/KeyEvent.hpp"

#include <GLFW/glfw3.h>

namespace Murloc {

	Windows_Window::Windows_Window(const WindowSpecification& specification)
	{
		m_Data.Width = specification.Width;
		m_Data.Height = specification.Height;
		m_Data.Title = specification.Title;
		m_Data.VSync = specification.VSync;
		Init();
	}

	void Windows_Window::Init()
	{
		glfwInit();

		m_NativeWindow = glfwCreateWindow(m_Data.Width, m_Data.Height, m_Data.Title.c_str(), nullptr, nullptr);

		MUR_CORE_INFO("Created window: {0}, {1}", m_Data.Width, m_Data.Height);

		glfwSetWindowUserPointer(m_NativeWindow, &m_Data);

		glfwSetFramebufferSizeCallback(m_NativeWindow, [](GLFWwindow* window, int width, int height) 
			{
				MUR_CORE_INFO("Resized: {0}, {1}", width, height);

				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				data.Width = width;
				data.Height = height;

				data.EventCallback(KeyEvent());
			});

		glfwMakeContextCurrent(m_NativeWindow);
	}


	void Windows_Window::OnUpdate()
	{
		glfwPollEvents();
		glfwSwapBuffers(m_NativeWindow);
	}

	void Windows_Window::SetEventCallback(const EventCallbackFn& fn)
	{
		m_Data.EventCallback = fn;
	}

	unsigned int Windows_Window::GetWidth() const
	{
		return m_Data.Width;
	}

	unsigned int Windows_Window::GetHeight() const
	{
		return m_Data.Height;
	}

	void Windows_Window::SetFullscreen(bool fullscreen)
	{
	}

	bool Windows_Window::FullScreenEnabled() const
	{
		return m_Data.Fullscreen;
	}

	void Windows_Window::SetVSync(bool vsync)
	{
	}

	void Windows_Window::VSyncEnabled(bool vsync)
	{

	}

	void* Windows_Window::GetNativeWindow() const
	{
		return m_NativeWindow;
	}

}