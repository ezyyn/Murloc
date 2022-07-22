#include <Pangolin.h>

#include <Pangolin/Core/EntryPoint.h>

#include "SandboxLayer.h"

class SandboxApp : public PG::Application 
{
public:
	SandboxApp(const PG::ApplicationSpecification& specification)
		: Application(specification) 
	{
	}

	void OnInit() override
	{
		PushLayer(new SandboxLayer());
	}

};


PG::Application* PG::CreateApplication(PG::ApplicationCommandLineArgs args)
{
	PG::ApplicationSpecification specification;

	return new SandboxApp(specification);
}

