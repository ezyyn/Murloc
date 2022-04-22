
--Murloc Dependencies

IncludeDir= {}
IncludeDir["GLFW"] = "%{wks.location}/Murloc/dependencies/GLFW/include"
IncludeDir["spdlog"] = "%{wks.location}/Murloc/dependencies/spdlog/include"
IncludeDir["ImGui"] = "%{wks.location}/Murloc/dependencies/imgui"

-- Vulkan
VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDir["shaderc"] =					"%{wks.location}/Murloc/dependencies/shaderc/include"
IncludeDir["SPIRV_Cross"] =				"%{wks.location}/Murloc/dependencies/SPIRV-Cross"
IncludeDir["VulkanSDK"] =				"%{VULKAN_SDK}/Include"

LibraryDir = {}

LibraryDir["VulkanSDK"] =				"%{VULKAN_SDK}/Lib"
LibraryDir["VulkanSDK_Debug"] =			"%{wks.location}/Murloc/dependencies/VulkanSDK/Lib"
LibraryDir["VulkanSDK_DebugDLL"] =		"%{wks.location}/Murloc/dependencies/VulkanSDK/Bin"

Library = {}
Library["Vulkan"] =						"%{LibraryDir.VulkanSDK}/vulkan-1.lib"
Library["VulkanUtils"] =				"%{LibraryDir.VulkanSDK}/VkLayer_utils.lib"

Library["ShaderC_Debug"] =				"%{LibraryDir.VulkanSDK_Debug}/shaderc_sharedd.lib"
Library["SPIRV_Cross_Debug"] =			"%{LibraryDir.VulkanSDK_Debug}/spirv-cross-cored.lib"
Library["SPIRV_Cross_GLSL_Debug"] =		"%{LibraryDir.VulkanSDK_Debug}/spirv-cross-glsld.lib"
Library["SPIRV_Tools_Debug"] =			"%{LibraryDir.VulkanSDK_Debug}/SPIRV-Toolsd.lib"

Library["ShaderC_Release"] =			"%{LibraryDir.VulkanSDK}/shaderc_shared.lib"
Library["SPIRV_Cross_Release"] =		"%{LibraryDir.VulkanSDK}/spirv-cross-core.lib"
Library["SPIRV_Cross_GLSL_Release"] =	"%{LibraryDir.VulkanSDK}/spirv-cross-glsl.lib"