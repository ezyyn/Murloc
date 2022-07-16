#include "SandboxLayer.hpp"

#include <glm/glm.hpp>

void SandboxLayer::OnAttach()
{
}

void SandboxLayer::OnDetach()
{
}

void SandboxLayer::OnUpdate(Murloc::Timestep& ts)
{
	Murloc::Renderer::BeginScene();
		Murloc::Renderer::DrawQuad(glm::vec3{ -1.0f,0.0f,0.0f }, { 1.0f,1.0f }, { 1.0f,0.0f,0.0f,1.0f });
		Murloc::Renderer::DrawQuad(glm::vec3{ 1.0f,0.0f,0.0f }, { 1.0f,1.0f }, { 1.0f,0.0f,0.0f,1.0f });
		Murloc::Renderer::DrawQuad(glm::vec3{ 0.0f,0.0f,0.0f }, { 1.0f,1.0f }, { 1.0f,0.0f,0.0f,1.0f });
		Murloc::Renderer::DrawQuad(glm::vec3{ 0.0f,1.0f,0.0f }, { 1.0f,1.0f }, { 1.0f,0.0f,0.0f,1.0f });
		Murloc::Renderer::DrawQuad(glm::vec3{ 0.0f,-1.0f,0.0f }, { 1.0f,1.0f }, { 1.0f,0.0f,0.0f,1.0f });
		Murloc::Renderer::DrawQuad(glm::vec3{ 0.0f,-1.0f,0.0f }, { 1.0f,1.0f }, { 1.0f,0.0f,0.0f,1.0f });
		Murloc::Renderer::DrawQuad(glm::vec3{ 1.0f,-1.0f,0.0f }, { 1.0f,1.0f }, { 1.0f,0.0f,0.0f,1.0f });
		Murloc::Renderer::DrawQuad(glm::vec3{ -1.0f,-1.0f,0.0f }, { 1.0f,1.0f }, { 1.0f,0.0f,0.0f,1.0f });
	Murloc::Renderer::EndScene();
}

void SandboxLayer::OnImGuiRender()
{
}

void SandboxLayer::OnEvent(Murloc::Event& e)
{
}
