#include "murpch.hpp"

#include "Application.hpp"

#ifdef MUR_PLATFORM_WINDOWS
	#include "Platform/Windows/Windows_Window.hpp"
#endif

#include "Murloc/Renderer/Renderer.hpp"

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
		Renderer::Shutdown();
	}

	void Application::Init()
	{
		m_Window = CreateScope<Windows_Window>();
		m_Window->SetEventCallback(MUR_BIND_FN(Application::OnEvent));

		Renderer::Init();
	}

	void Application::Run()
	{
		float lastFrame = 0.0f;

		OnInit();
		while (m_Running) {

			float currentFrame = m_Window->GetTime();

			Timestep ts = currentFrame - lastFrame;

			lastFrame = currentFrame;

			m_Window->ProcessEvents();

			if (!m_Minimized) 
			{
				for (Layer* layer : m_LayerStack)
				{
					layer->OnUpdate(ts);
				}
				Renderer::BeginFrame();
				Renderer::EndFrame();

			}
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
		m_Minimized = false;
		if (e.GetWidth() == 0 || e.GetHeight() == 0) 
		{
			m_Minimized = true;

			return false;
		}

		Renderer::OnResize(e.GetWidth(), e.GetHeight());

		return false;
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;

		return true;
	}

}