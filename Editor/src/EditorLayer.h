#pragma once

#include <Pangolin/Core/Layer.h>
#include <Pangolin/Entities/Scene.h>
#include <Pangolin/Renderer/EditorCamera.h>

#include <glm/glm.hpp>

#include "Panels/SceneHierarchyPanel.h"

namespace PG {
	
	enum class SceneMode {
		Edit = 0,
		Play
	};
	
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
		
		SceneMode m_SceneMode{ SceneMode::Edit };
		
		SceneHierarchyPanel m_SceneHierarchyPanel;
		Ref<Scene> m_EditorScene;

		Ref<Scene> m_ActiveScene;
		glm::vec2 m_ViewportSize;
	};
}