#pragma once

#include "Application.h"
#include "Common.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

#ifdef PG_PLATFORM_WINDOWS 
	
extern PG::Application* PG::CreateApplication(PG::ApplicationCommandLineArgs args);

int main(int argc, char* argv[])
{
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);

	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// SPIR-V Compiler leak
	// _CrtSetBreakAlloc(774);
	{
		PG::Log::Init();

		auto app = PG::CreateApplication({ argc, argv });
		app->Run();
		delete app;

		PG::Log::Shutdown();
	}
}

#endif