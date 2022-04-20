
--Murloc Dependencies

VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDir= {}
IncludeDir["GLFW"] = "%{wks.location}/Murloc/dependencies/GLFW/include"
IncludeDir["spdlog"] = "%{wks.location}/Murloc/dependencies/spdlog/include"