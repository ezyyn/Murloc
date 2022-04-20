#pragma once

#include "Application.hpp"
#include "Common.hpp"

#ifdef MUR_PLATFORM_WINDOWS 
	
extern Murloc::Application* Murloc::CreateApplication(Murloc::ApplicationCommandLineArgs args);

int main(int argc, char* argv[])
{
	Murloc::Log::Init();

	auto app = Murloc::CreateApplication({ argc, argv });
	app->Run();
	delete app;

	Murloc::Log::Shutdown();
}

#endif