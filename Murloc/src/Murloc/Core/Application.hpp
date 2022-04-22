#pragma once

#include "Window.hpp"

#include "Murloc/Event/ApplicationEvent.hpp"
#include "Murloc/Event/MouseEvent.hpp"
#include "Murloc/Core/LayerStack.hpp"

namespace Murloc {

	struct ApplicationSpecification 
	{
		std::string Title = "Murloc App";
		uint32_t Width = 1280;
		uint32_t Height = 720;
		bool Decorated = true;
		bool Fullscreen = false;
		bool VSync = true;
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

	class Application {

	public:
		Application(const ApplicationSpecification& specification = ApplicationSpecification());
		~Application();

		virtual void OnInit() = 0;

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

		LayerStack m_LayerStack;

		bool m_Minimized{ false };

		ApplicationSpecification m_Specification;

		static Application* s_Instance;
	};
	
	// To be defined in client
	Application* CreateApplication(ApplicationCommandLineArgs args);
}