#include "murpch.hpp"

#include "Windows_Window.hpp"

#include "Murloc/Event/KeyEvent.hpp"
#include "Murloc/Event/MouseEvent.hpp"
#include "Murloc/Event/ApplicationEvent.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Murloc {

	static void ErrorCallback(int error_code, const char* description) 
	{
		MUR_CORE_ERROR("Error[{0}]: {1}", error_code, description);
	}

	Windows_Window::Windows_Window(const WindowSpecification& specification)
	{
		m_Data.Width = specification.Width;
		m_Data.Height = specification.Height;
		m_Data.Title = specification.Title;
		m_Data.VSync = specification.VSync;
		Init();
	}

	Windows_Window::~Windows_Window()
	{
		glfwDestroyWindow(m_NativeWindow);
		glfwTerminate();
	}

	void Windows_Window::Init()
	{
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // For now...

		m_NativeWindow = glfwCreateWindow(m_Data.Width, m_Data.Height, m_Data.Title.c_str(), nullptr, nullptr);

		MUR_CORE_INFO("Created window: {0}, {1}", m_Data.Width, m_Data.Height);

		glfwSetWindowUserPointer(m_NativeWindow, &m_Data);

		glfwSetErrorCallback(ErrorCallback);

		glfwSetFramebufferSizeCallback(m_NativeWindow, [](GLFWwindow* window, int width, int height) 
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				data.Width = width;
				data.Height = height;

				WindowResizeEvent _event(width, height);

				data.EventCallback(_event);
			});

		glfwSetWindowCloseCallback(m_NativeWindow, [](GLFWwindow* window)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				WindowCloseEvent _event;
				data.EventCallback(_event);
			});
		glfwSetKeyCallback(m_NativeWindow, [](GLFWwindow* window, int key, int scancode, int action, int mods)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				switch (action)
				{
				case GLFW_PRESS:
				{
					KeyPressedEvent _event(key, 0);
					data.EventCallback(_event);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent _event(key);
					data.EventCallback(_event);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent _event(key, 0);
					data.EventCallback(_event);
					break;
				}
				}
			});

		glfwSetMouseButtonCallback(m_NativeWindow, [](GLFWwindow* window, int button, int action, int mods)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				switch (action)
				{
				case GLFW_PRESS:
				{
					MouseButtonPressedEvent _event(button);
					data.EventCallback(_event);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent _event(button);
					data.EventCallback(_event);
					break;
				}
				}
			});
		glfwSetCharCallback(m_NativeWindow, [](GLFWwindow* window, unsigned int keycode)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				KeyTypedEvent _event(keycode);
				data.EventCallback(_event);
			});

		glfwSetScrollCallback(m_NativeWindow, [](GLFWwindow* window, double xOffset, double yOffset)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				MouseScrolledEvent _event((float)xOffset, (float)yOffset);
				data.EventCallback(_event);
			});

		glfwSetCursorPosCallback(m_NativeWindow, [](GLFWwindow* window, double x, double y)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				MouseMovedEvent _event((float)x, (float)y);
				data.EventCallback(_event);
			});
	}


	void Windows_Window::ProcessEvents()
	{
		glfwPollEvents();
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

	float Windows_Window::GetTime() const
	{
		return (float)glfwGetTime();
	}

}