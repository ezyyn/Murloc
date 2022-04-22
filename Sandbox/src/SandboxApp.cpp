#include <Murloc.hpp>

#include <Murloc/Core/EntryPoint.hpp>

class SandboxApp : public Murloc::Application 
{
public:
	SandboxApp(const Murloc::ApplicationSpecification& specification)
		: Application(specification) 
	{
	}

	void OnInit() override
	{

	}

};


Murloc::Application* Murloc::CreateApplication(Murloc::ApplicationCommandLineArgs args)
{
	Murloc::ApplicationSpecification specification;

	specification.Width = 800;
	specification.Height = 800;

	return new SandboxApp(specification);
}

