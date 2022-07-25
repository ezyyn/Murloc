
--Pangolin Editor

project "Editor"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-obj/" .. outputdir .. "/%{prj.name}")

	files 
	{
		"src/**.h",
		"src/**.cpp"
	}
	includedirs 
	{
		"%{wks.location}/Pangolin/src",
		"%{wks.location}/Pangolin/dependencies/imgui",
		"%{wks.location}/Pangolin/dependencies/spdlog/include",
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}"
		--"%{IncludeDir.ImGuizmo}"
	}
	links 
	{
		"Pangolin",
		"ImGui"
	}

	filter "system:windows"
		systemversion "latest"

		defines 
		{
			"PG_PLATFORM_WINDOWS"
		}
		filter "configurations:Debug"
			defines "PG_DEBUG"
			runtime "Debug"
			symbols "on"

			postbuildcommands
			{
				"{COPYDIR} \"%{LibraryDir.VulkanSDK_DebugDLL}\" \"%{cfg.targetdir}\""
			}

		filter "configurations:Release"
			defines "PG_RELEASE"
			runtime "Release"
			optimize "on"
			