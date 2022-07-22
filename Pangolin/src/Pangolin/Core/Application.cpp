#include "pgpch.h"

#include "Application.h"

#ifdef PG_PLATFORM_WINDOWS
	#include "Platform/Windows/Windows_Window.h"
#endif
#include "Pangolin/Renderer/RenderManager.h"
#include "Pangolin/Scripting/ScriptEngine.h"

#include "Pangolin/ImGui/ImGuiLayer.h"

namespace PG {

	Application* Application::s_Instance{ nullptr };

	Application::Application(const ApplicationSpecification& specification /*= ApplicationSpecification()*/)
		: m_Specification(specification)
	{
		PG_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		Init();
	}

	void Application::Init()
	{
		WindowSpecification windowSpecification{};
		windowSpecification.Fullscreen = m_Specification.Fullscreen;
		windowSpecification.Width = m_Specification.Width;
		windowSpecification.Height = m_Specification.Height;
		windowSpecification.VSync = m_Specification.VSync;
		windowSpecification.Decorated = m_Specification.Decorated;
		windowSpecification.Title = m_Specification.Title;
		
		m_Window = CreateScope<Windows_Window>(windowSpecification);
		m_Window->Init();
		m_Window->SetEventCallback(PG_BIND_FN(Application::OnEvent));

		if (m_Specification.EnableImGui) {
			m_ImGuiLayer = new ImGuiLayer;
			PushOverLay(m_ImGuiLayer);
		}

		RenderManager::Init();

		//ScriptEngine::Init();
	}

	Application::~Application()
	{
		for (Layer* layer : m_LayerStack)
		{
			layer->OnDetach();
		}

		m_Window.reset();
		//ScriptEngine::Shutdown();

		RenderManager::Shutdown();
	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverLay(Layer* layer)
	{
		m_LayerStack.PushOverlay(layer);
		layer->OnAttach();
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
				
				m_Window->AcquireNewSwapchainFrame();

				if (m_Specification.EnableImGui) 
				{
					m_ImGuiLayer->Begin();

					for (Layer* layer : m_LayerStack)
					{
						layer->OnImGuiRender();
					}

					m_ImGuiLayer->End();
				}

				m_Window->SwapBuffers();
			}
		}

	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);

		dispatcher.Dispatch<WindowResizeEvent>(PG_BIND_FN(Application::OnWindowResize));
		dispatcher.Dispatch<WindowCloseEvent>(PG_BIND_FN(Application::OnWindowClose));

		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it) {
			
			if (e.Handled)
				break;

			(*it)->OnEvent(e);
		}
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		m_Specification.Width = e.GetWidth();
		m_Specification.Height = e.GetHeight();

		m_Minimized = false;
		if (e.GetWidth() == 0 || e.GetHeight() == 0) 
		{
			m_Minimized = true;
		}

		if (m_Minimized == false)
			RenderManager::OnResize(m_Specification.Width, m_Specification.Height);

		return false;
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;

		return true;
	}
}