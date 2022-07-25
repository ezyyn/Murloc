#pragma once 

#include <Pangolin/Entities/Scene.h>

#include <Pangolin/Event/Event.h>

#include <Pangolin/Entities/Entity.h>

namespace PG {
	
	class SceneHierarchyPanel {
	public:
		SceneHierarchyPanel() = default;
		~SceneHierarchyPanel() = default;

		void OnImGuiRender();
		void OnEvent(Event& e);
		void DrawComponents(Entity entity);
	
		inline void SetCurrentScene(const Ref<Scene>& scene) { m_CurrentScene = scene; }

	private:
		void DrawEntityNode(Entity entity);

		Ref<Scene> m_CurrentScene;
		Entity m_SelectedEntity{ };
	};
	
}