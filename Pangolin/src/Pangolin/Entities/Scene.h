#pragma once

#include "Pangolin/Core/Time.h"

#include "Pangolin/Renderer/EditorCamera.h"
#include "Pangolin/Renderer/SceneRenderer.h"

#include <entt.hpp>

namespace PG {
	
	class Entity;

	using SceneID = void*;
	
	struct SceneStatistics
	{
		uint32_t Entities;
		uint32_t RenderedEntities;
	};

	class Scene {
	public:
		Scene();
		~Scene();

		void OnUpdateRuntime(Timestep& ts);
		void OnUpdateEditor(Timestep& ts, const EditorCamera& camera);

		Entity CreateEntity(const std::string& name);
		void DestroyEntity(const Entity& entity);

		template<typename... Components>
		inline auto GetAllEntitiesWith()
		{
			return m_Registry.view<Components...>();
		}

		inline const SceneStatistics& GetStatistics() const { return m_Statistics; }

		SceneID GetRenderedScene() const;
	private:
		Ref<SceneRenderer> m_Renderer;

		entt::registry m_Registry;

		SceneStatistics m_Statistics{};

		friend class SceneHierarchyPanel;
		friend class Entity;
	};
}