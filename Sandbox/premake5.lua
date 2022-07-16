
-- Murloc Sandbox

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
        "%{wks.location}/Murloc/src",
        "%{wks.location}/Murloc/dependencies/spdlog/include",
        "%{IncludeDir.glm}"
    }

    links 
	{
        "Murloc"
    }

     
	filter "system:windows"
    systemversion "latest"

    defines 
    {
        "MUR_PLATFORM_WINDOWS"
    }

filter "configurations:Debug"
    defines "MUR_DEBUG"
    runtime "Debug"
    symbols "on"

    --postbuildcommands
	--{
	--	"{COPYDIR} \"%{LibraryDir.VulkanSDK_DebugDLL}\" \"%{cfg.targetdir}\""
	--}

filter "configurations:Release"
    defines "MUR_RELEASE"
    runtime "Release"
    optimize "on"