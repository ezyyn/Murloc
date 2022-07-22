
-- Pangolin Sandbox

project "Sandbox"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"
    floatingpoint "Fast"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-obj/" .. outputdir .. "/%{prj.name}")

    files 
    {   
        "src/**.h",
        "src/**.hpp",
        "src/**.cpp"
    }

    includedirs 
	{
		"src",
        "%{wks.location}/Pangolin/src",
        "%{wks.location}/Pangolin/dependencies/spdlog/include",
        "%{IncludeDir.glm}",
        "%{IncludeDir.ImGui}"
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
        PG_PLATFORM_WINDOWS
    }

filter "configurations:Debug"
    defines "PG_DEBUG"
    runtime "Debug"
    symbols "on"

    --postbuildcommands
	--{
	--	"{COPYDIR} \"%{LibraryDir.VulkanSDK_DebugDLL}\" \"%{cfg.targetdir}\""
	--}

filter "configurations:Release"
    defines "PG_RELEASE"
    runtime "Release"
    optimize "on"