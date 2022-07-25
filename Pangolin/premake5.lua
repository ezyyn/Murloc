-- Pangolin Engine

project "Pangolin"
    kind "StaticLib"
    cppdialect "C++17"
	staticruntime "off"
	floatingpoint "Fast"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-obj/" .. outputdir .. "/%{prj.name}")

    pchheader "pgpch.h"
	pchsource "src/pgpch.cpp"

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
        "%{IncludeDir.VulkanSDK}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.stb}",
        "%{IncludeDir.entt}",
     --   "%{IncludeDir.yaml_cpp}",
       -- "%{IncludeDir.mono}"
    }

    links {
        "GLFW",
        "ImGui",
       -- "yaml-cpp",
        "%{Library.Vulkan}",
       -- "%{Library.mono}"
    }
    
	filter "system:windows"
        systemversion "latest"

        defines {
            "PG_PLATFORM_WINDOWS"
        }

        links {
            "%{Library.WinSock}",
            "%{Library.WinMM}",
            "%{Library.WinVersion}",
            "%{Library.WinBcryp}"
        }

    filter "configurations:Debug"
        defines "PG_DEBUG"
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
        defines "PG_RELEASE"
        runtime "Release"
        optimize "on"

        links
        {
            "%{Library.ShaderC_Release}",
            "%{Library.SPIRV_Cross_Release}",
            "%{Library.SPIRV_Cross_GLSL_Release}"
        }

