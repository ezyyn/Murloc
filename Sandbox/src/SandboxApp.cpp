#include <Murloc.hpp>

#include <Murloc/Core/EntryPoint.hpp>

#include "SandboxLayer.hpp"

class SandboxApp : public Murloc::Application 
{
public:
	SandboxApp(const Murloc::ApplicationSpecification& specification)
		: Application(specification) 
	{
	}

	void OnInit() override
	{
		PushLayer(new SandboxLayer());
	}

};


Murloc::Application* Murloc::CreateApplication(Murloc::ApplicationCommandLineArgs args)
{
	Murloc::ApplicationSpecification specification;

	return new SandboxApp(specification);
}

