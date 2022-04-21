#pragma once

#include "Window.hpp"

#include "Murloc/Event/ApplicationEvent.hpp"
#include "Murloc/Event/MouseEvent.hpp"

namespace Murloc {

	struct ApplicationSpecification {
		uint32_t Width;
		uint32_t Height;
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
		Application(const Murloc::ApplicationSpecification& specification = ApplicationSpecification());
		~Application();

		void Init();

		void Run();

		bool OnEvent(Event& e);

		bool OnWindowResize(WindowResizeEvent& e);
		bool OnWindowClose(WindowCloseEvent& e);
	private:
		Scope<Window> m_Window{ nullptr };

		bool m_Running{ true };

		bool m_Minimized{ false };

		ApplicationSpecification m_Specification;
	};
	
	// To be defined in client
	Application* CreateApplication(ApplicationCommandLineArgs args);
}