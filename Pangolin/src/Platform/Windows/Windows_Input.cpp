#include "pgpch.h"
#include "Pangolin/Core/Input.h"

#include "Pangolin/Core/Application.h"
#include <GLFW/glfw3.h>

namespace PG
{
	bool Input::IsKeyPressed(int keycode)
	{
		auto window = static_cast<GLFWwindow*>(Application::Get()->GetWindow()->GetNativeWindow());
		auto state = glfwGetKey(window, keycode);
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool Input::IsKeyReleased(int keycode)
	{
		auto window = static_cast<GLFWwindow*>(Application::Get()->GetWindow()->GetNativeWindow());

		auto state = glfwGetKey(window, keycode);
		return state == GLFW_RELEASE;
	}

	bool Input::IsMouseButtonPressed(int button)
	{
		auto window = static_cast<GLFWwindow*>(Application::Get()->GetWindow()->GetNativeWindow());

		auto state = glfwGetMouseButton(window, button);
		return state == GLFW_PRESS;
	}

	float Input::GetMouseX()
	{
		return (float)GetMousePosition().X;
	}

	float Input::GetMouseY()
	{
		return (float)GetMousePosition().Y;
	}

	Input::MousePosition Input::GetMousePosition()
	{
		auto window = static_cast<GLFWwindow*>(Application::Get()->GetWindow()->GetNativeWindow());

		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		return{ (float)xpos, (float)ypos };
	}
}