#include "murpch.hpp"

#include "Application.hpp"

#ifdef MUR_PLATFORM_WINDOWS
	#include "Platform/Windows/Windows_Window.hpp"
#endif

#include "Platform/Vulkan/Vulkan.hpp"

namespace Murloc {

	Murloc::Application* Application::s_Instance{ nullptr };

	Application::Application(const ApplicationSpecification& specification /*= ApplicationSpecification()*/)
		: m_Specification(specification)
	{
		MUR_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		Init();
	}

	Application::~Application()
	{
		Vulkan::Shutdown();
	}

	void Application::Init()
	{
		m_Window = CreateScope<Windows_Window>();
		m_Window->SetEventCallback(MUR_BIND_FN(Application::OnEvent));

		Vulkan::Init(m_Window->GetContext());
	}

	void Application::Run()
	{
		float lastFrame = 0.0f;

		OnInit();
		while (m_Running) {

			float currentFrame = m_Window->GetTime();

			Timestep ts = currentFrame - lastFrame;

			lastFrame = currentFrame;

			if (!m_Minimized) 
			{
				for (Layer* layer : m_LayerStack)
				{
					layer->OnUpdate(ts);
				}
			}
			m_Window->OnUpdate();
		}
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);

		dispatcher.Dispatch<WindowResizeEvent>(MUR_BIND_FN(Application::OnWindowResize));
		dispatcher.Dispatch<WindowCloseEvent>(MUR_BIND_FN(Application::OnWindowClose));

		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it) {
			
			if (e.Handled)
				break;

			(*it)->OnEvent(e);
		}
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		if (e.GetWidth() == 0 || e.GetHeight() == 0) 
		{
			m_Minimized = true;

			return false;
		}

		return false;
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;

		return true;
	}

}