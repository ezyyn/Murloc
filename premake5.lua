--Murloc Workspace

include "./dependencies/premake/premake_customization/solution_items.lua"
include "Dependencies.lua"

workspace "Murloc" 
    architecture "x86_64"
	startproject "Sandbox"
	toolset "v142"

	configurations {
	    "Debug",
		"Release",
    }

	flags {
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}";

group "Dependencies"
	include "Murloc/dependencies/GLFW"
group ""
	include "Sandbox"
	include "Murloc"
