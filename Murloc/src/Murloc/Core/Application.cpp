#include "murpch.hpp"

#include "Application.hpp"

#include <iostream>

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
		MUR_CORE_INFO("HELLO");
		return false;
	}

}