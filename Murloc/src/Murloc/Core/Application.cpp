#include "murpch.hpp"

#include "Application.hpp"

#ifdef MUR_PLATFORM_WINDOWS
	#include "Platform/Windows/Windows_Window.hpp"
#endif
namespace Murloc {

	Application::Application(const ApplicationSpecification& specification)
		: m_Specification(specification)
	{
		Init();
	}

	Application::~Application()
	{
	}

	void Application::Init()
	{
		m_Window = CreateScope<Windows_Window>();
		m_Window->SetEventCallback(MUR_BIND_FN(Application::OnEvent));
	}

	void Application::Run()
	{
		while (m_Running) {

			m_Window->OnUpdate();
		}
	}


	bool Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);

		dispatcher.Dispatch<WindowResizeEvent>(MUR_BIND_FN(Application::OnWindowResize));
		dispatcher.Dispatch<WindowCloseEvent>(MUR_BIND_FN(Application::OnWindowClose));
		return false;
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