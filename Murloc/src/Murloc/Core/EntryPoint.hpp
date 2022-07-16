#pragma once

#include "Application.hpp"
#include "Common.hpp"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

#ifdef MUR_PLATFORM_WINDOWS 
	
extern Murloc::Application* Murloc::CreateApplication(Murloc::ApplicationCommandLineArgs args);

int main(int argc, char* argv[])
{
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);

	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// SPIR-V Compiler leak
	// _CrtSetBreakAlloc(774);
	{
		Murloc::Log::Init();

		auto app = Murloc::CreateApplication({ argc, argv });
		app->Run();
		delete app;

		Murloc::Log::Shutdown();
	}
}

#endif