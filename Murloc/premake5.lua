-- Murloc Engine

project "Murloc"
    kind "StaticLib"
    cppdialect "C++17"
	staticruntime "off"
	floatingpoint "Fast"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-obj/" .. outputdir .. "/%{prj.name}")

    pchheader "murpch.hpp"
	pchsource "src/murpch.cpp"

    files {

        "src/**.hpp",
        "src/**.h",
        "src/**.cpp",
    }

    defines {
        "GLFW_INCLUDE_NONE",
        "_CRT_SECURE_NO_WARNINGS"
    }

    includedirs {
        "src",
        "%{IncludeDir.spdlog}",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.VulkanSDK}"
    }

    links {
        "GLFW",
        "ImGui",
        "%{Library.Vulkan}"
    }

    
	filter "system:windows"
        systemversion "latest"

        defines {
            "MUR_PLATFORM_WINDOWS"
        }

    filter "configurations:Debug"
        defines "MUR_DEBUG"
        runtime "Debug"
        symbols "on"

        postbuildcommands
        {
            "{COPYDIR} \"%{LibraryDir.VulkanSDK_DebugDLL}\" \"%{cfg.targetdir}\""
        }

        links
        {
           "%{Library.ShaderC_Debug}",
           "%{Library.SPIRV_Cross_Debug}",
           "%{Library.SPIRV_Cross_GLSL_Debug}"
        }

    filter "configurations:Release"
        defines "MUR_RELEASE"
        runtime "Release"
        optimize "on"

        --links
        --{
          --  "%{Library.ShaderC_Release}",
            --"%{Library.SPIRV_Cross_Release}",
           -- "%{Library.SPIRV_Cross_GLSL_Release}"
       -- }

