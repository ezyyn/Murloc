--Pangolin Workspace

include "./dependencies/premake/premake_customization/solution_items.lua"
include "Dependencies.lua"

workspace "Pangolin" 
    architecture "x86_64"
	startproject "Editor"
	toolset "v143"

	configurations {
	    "Debug",
		"Release",
    }

	flags {
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}";

group "Dependencies"
	include "Pangolin/dependencies/GLFW"
	include "Pangolin/dependencies/ImGui"
group "Core"
	include "Pangolin"

group "Core-Tools"
	include "Editor"