#include "Pangolin/Core/EntryPoint.h"

#include "EditorLayer.h"

namespace PG {

	class EditorApplication : public Application {
	public:
		EditorApplication(const ApplicationSpecification& specification)
			: Application(specification)
		{
		}
		~EditorApplication()
		{
		}

		void OnInit() override
		{
			PushLayer(new EditorLayer);
		}

	};

	Application* CreateApplication(ApplicationCommandLineArgs args) {

		ApplicationSpecification specification{};
		specification.Title = "Pangolin Editor";
		specification.Width = 1600;
		specification.Height = 900;
		specification.VSync = true;
		specification.EnableImGui = true;
		specification.Decorated = false;
		//specification.StartMaximized = false;

		return new EditorApplication(specification);
	}
}