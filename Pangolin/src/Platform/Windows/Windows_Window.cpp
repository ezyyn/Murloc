#include "pgpch.h"

#include "Windows_Window.h"

#include "Pangolin/Event/KeyEvent.h"
#include "Pangolin/Event/MouseEvent.h"
#include "Pangolin/Event/ApplicationEvent.h"

#include "Pangolin/Renderer/Vulkan/Swapchain.h"
#include "Pangolin/Renderer/Vulkan/VulkanContext.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace PG {

	static void ErrorCallback(int error_code, const char* description) 
	{
		PG_CORE_ERROR("Error[{0}]: {1}", error_code, description);
	}

	Windows_Window::Windows_Window(const WindowSpecification& specification)
	{
		m_Data.Width = specification.Width;
		m_Data.Height = specification.Height;
		m_Data.Title = specification.Title;
		m_Data.VSync = specification.VSync;
	}

	Windows_Window::~Windows_Window()
	{
		VulkanContext::Shutdown();

		glfwDestroyWindow(m_NativeWindow);
		glfwTerminate();
	}

	void Windows_Window::Init()
	{
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // For now...

		m_NativeWindow = glfwCreateWindow(m_Data.Width, m_Data.Height, m_Data.Title.c_str(), nullptr, nullptr);
		PG_CORE_INFO("Created window: {0}, {1}", m_Data.Width, m_Data.Height);

		glfwSetErrorCallback(ErrorCallback);
		MapKeyEvents();

		// vkInstance, vkSurface, vkDevice, vkPhysicalDevice
		VulkanContext::Init();

		m_Swapchain = CreateRef<Swapchain>();

		m_Data.Swapchain = (void*)m_Swapchain.get(); // BEWARE
	}


	void Windows_Window::AcquireNewSwapchainFrame()
	{
		m_Swapchain->BeginFrame();
	}

	void Windows_Window::SwapBuffers()
	{
		m_Swapchain->SwapFrame();
	}

	void Windows_Window::MapKeyEvents()
	{
		glfwSetWindowUserPointer(m_NativeWindow, &m_Data);

		glfwSetFramebufferSizeCallback(m_NativeWindow, [](GLFWwindow* window, int width, int height)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				data.Width = width;
				data.Height = height;

				WindowResizeEvent _event(width, height);
				if (data.Width != 0 && data.Height != 0) {
					// TODO: Not ideal, mabye rework this

					Swapchain* swapChain = (Swapchain*)data.Swapchain;
					swapChain->Invalidate();
				}

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

	void Windows_Window::SetWindowTitle(const char* title)
	{
		glfwSetWindowTitle(m_NativeWindow, title);
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
		m_Data.VSync = vsync;
	}

	bool Windows_Window::VSyncEnabled()
	{
		return m_Data.VSync;
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