#include "murpch.hpp"

#include "Application.hpp"

#ifdef MUR_PLATFORM_WINDOWS
	#include "Platform/Windows/Windows_Window.hpp"
#endif
#include "Murloc/Renderer/Renderer.hpp"
#include "Murloc/Scripting/ScriptEngine.h"

#include "Murloc/ImGui/ImGuiLayer.hpp"

namespace Murloc {

	Application* Application::s_Instance{ nullptr };

	Application::Application(const ApplicationSpecification& specification /*= ApplicationSpecification()*/)
		: m_Specification(specification)
	{
		MUR_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		Init();
	}

	void Application::Init()
	{
		m_Window = CreateScope<Windows_Window>();
		m_Window->Init();
		m_Window->SetEventCallback(MUR_BIND_FN(Application::OnEvent));

		Renderer::Init();
		//ScriptEngine::Init();
	}

	Application::~Application()
	{
		for (Layer* layer : m_LayerStack)
		{
			layer->OnDetach();
		}

		//ScriptEngine::Shutdown();
		Renderer::Shutdown();
	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}
//#define IMGUI

#ifdef IMGUI
	static ImGuiLayer* s_VulkanImGuiLayer;
#endif

	void Application::Run()
	{
		float lastFrame = 0.0f;
#ifdef IMGUI
		s_VulkanImGuiLayer = new ImGuiLayer();

		s_VulkanImGuiLayer->OnAttach();
#endif
		OnInit();
		while (m_Running) {

			float currentFrame = m_Window->GetTime();
			Timestep ts = currentFrame - lastFrame;
			lastFrame = currentFrame;

			std::string& title = m_Specification.Title + + " Last frame: " + std::to_string(ts.GetMs()) + "ms";
			m_Window->SetWindowTitle(title.c_str());

			m_Window->ProcessEvents();

			m_Window->AcquireNewSwapchainFrame();

			if (!m_Minimized)
			{
#ifdef IMGUI
				s_VulkanImGuiLayer->Begin();
#endif
				for (Layer* layer : m_LayerStack)
				{
					layer->OnUpdate(ts);
				}
#ifdef IMGUI
				s_VulkanImGuiLayer->End();
#endif
			}

			m_Window->SwapBuffers();
		}
#ifdef IMGUI
		s_VulkanImGuiLayer->OnDetach();
		delete s_VulkanImGuiLayer;
#endif
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
		m_Specification.Width = e.GetWidth();
		m_Specification.Height = e.GetHeight();

		m_Minimized = false;
		if (e.GetWidth() == 0 || e.GetHeight() == 0) 
		{
			m_Minimized = true;
		}

		// Renderer::OnResize(e.GetWidth(), e.GetHeight());

		return false;
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;

		return true;
	}
}