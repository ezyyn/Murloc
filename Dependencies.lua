
--Pangolin Dependencies

IncludeDir= {}
IncludeDir["GLFW"] = "%{wks.location}/Pangolin/dependencies/GLFW/include"
IncludeDir["spdlog"] = "%{wks.location}/Pangolin/dependencies/spdlog/include"
IncludeDir["ImGui"] = "%{wks.location}/Pangolin/dependencies/imgui"
IncludeDir["glm"] = "%{wks.location}/Pangolin/dependencies/glm"
IncludeDir["mono"] = "%{wks.location}/Pangolin/dependencies/mono/include"
IncludeDir["stb"] = "%{wks.location}/Pangolin/dependencies/stb"
IncludeDir["yaml_cpp"] = "%{wks.location}/Pangolin/dependencies/yaml-cpp/include"
IncludeDir["entt"] = "%{wks.location}/Pangolin/dependencies/entt/include"

-- Vulkan
VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDir["shaderc"] =					"%{wks.location}/Pangolin/dependencies/shaderc/include"
IncludeDir["SPIRV_Cross"] =				"%{wks.location}/Pangolin/dependencies/SPIRV-Cross"
IncludeDir["VulkanSDK"] =				"%{VULKAN_SDK}/Include"

LibraryDir = {}

LibraryDir["VulkanSDK"] =				"%{VULKAN_SDK}/Lib"
LibraryDir["VulkanSDK_Debug"] =			"%{wks.location}/Pangolin/dependencies/VulkanSDK/Lib"
LibraryDir["VulkanSDK_DebugDLL"] =		"%{wks.location}/Pangolin/dependencies/VulkanSDK/Bin"
LibraryDir["Mono"] =		            "%{wks.location}/Pangolin/dependencies/mono/lib/%{cfg.buildcfg}"

Library = {}
Library["Vulkan"] =						"%{LibraryDir.VulkanSDK}/vulkan-1.lib"
Library["VulkanUtils"] =				"%{LibraryDir.VulkanSDK}/VkLayer_utils.lib"

Library["mono"] =				        "%{LibraryDir.Mono}/libmono-static-sgen.lib"

Library["ShaderC_Debug"] =				"%{LibraryDir.VulkanSDK_Debug}/shaderc_sharedd.lib"
Library["SPIRV_Cross_Debug"] =			"%{LibraryDir.VulkanSDK_Debug}/spirv-cross-cored.lib"
Library["SPIRV_Cross_GLSL_Debug"] =		"%{LibraryDir.VulkanSDK_Debug}/spirv-cross-glsld.lib"
Library["SPIRV_Tools_Debug"] =			"%{LibraryDir.VulkanSDK_Debug}/SPIRV-Toolsd.lib"

Library["ShaderC_Release"] =			"%{LibraryDir.VulkanSDK}/shaderc_shared.lib"
Library["SPIRV_Cross_Release"] =		"%{LibraryDir.VulkanSDK}/spirv-cross-core.lib"
Library["SPIRV_Cross_GLSL_Release"] =	"%{LibraryDir.VulkanSDK}/spirv-cross-glsl.lib"
--Windows
Library["WinSock"] = "Ws2_32.lib"
Library["WinMM"] = "Winmm.lib"
Library["WinVersion"] = "Version.lib"
Library["WinBcryp"] = "Bcrypt.lib"