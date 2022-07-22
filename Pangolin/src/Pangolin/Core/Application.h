#pragma once

#include "Window.h"

#include "Pangolin/Event/ApplicationEvent.h"
#include "Pangolin/Event/MouseEvent.h"
#include "Pangolin/Core/LayerStack.h"

namespace PG {

	struct ApplicationSpecification 
	{
		std::string Title = "Pangolin App";
		uint32_t Width = 1280;
		uint32_t Height = 720;
		bool Decorated = true;
		bool Fullscreen = false;
		bool VSync = true;
		bool EnableImGui = true;
	};
	struct ApplicationCommandLineArgs
	{
		int Count = 0;
		char** Args = nullptr;

		const char* operator[](int index) const
		{
			return Args[index];
		}
	};

	class ImGuiLayer;

	class Application {

	public:
		Application(const ApplicationSpecification& specification = ApplicationSpecification());
		~Application();

		virtual void OnInit() = 0;
		void PushLayer(Layer* layer);
		void PushOverLay(Layer* layer);

		void Run();

		void OnEvent(Event& e);

		bool OnWindowResize(WindowResizeEvent& e);
		bool OnWindowClose(WindowCloseEvent& e);


		static inline Application* Get() { return s_Instance; }

		const Scope<Window>& GetWindow() { return m_Window; }

		const ApplicationSpecification& GetSpecification() {
			return m_Specification;
		}
	private:
		void Init();

		Scope<Window> m_Window{ nullptr };
		bool m_Running{ true };

		ImGuiLayer* m_ImGuiLayer{ nullptr };
		LayerStack m_LayerStack;

		bool m_Minimized{ false };

		ApplicationSpecification m_Specification;

		static Application* s_Instance;
	};
	
	// To be defined in client
	Application* CreateApplication(ApplicationCommandLineArgs args);
}