#pragma once

#include "Murloc/Core/Window.hpp"

struct GLFWwindow;

namespace Murloc {

	class VulkanContext {
	public:

		VulkanContext(GLFWwindow* window);

		std::vector<const char*> GetExtensions();
	private:
		GLFWwindow* m_Window;
		std::vector<const char*> m_Extensions;
	};

}