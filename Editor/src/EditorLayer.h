#pragma once

#include <Pangolin.h>
#include <glm/glm.hpp>

namespace PG {

	class EditorLayer : public Layer {
	public:
		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate(Timestep& ts) override;
		void OnImGuiRender() override;
		void OnEvent(Event& e) override;
	private:
		void ImGui_BeginDockspace();

		EditorCamera m_EditorCamera;

		glm::vec2 m_ViewportSize{};
	};
}