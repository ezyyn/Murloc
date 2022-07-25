#pragma once

#include "Pangolin/Core/Common.h"
#include "Components.h"

#include "Scene.h"

#include <entt.hpp>

namespace PG {

	class Entity
	{
	public:
		Entity() = default;
		Entity(const Entity& other)
		{
			m_Handle = other.m_Handle;
			m_Scene = other.m_Scene;
		}

		Entity(entt::entity id, Scene* scene)
			: m_Handle(id), m_Scene(scene) {}

		~Entity() {}

		Entity(Entity&& other) noexcept
		{
			m_Handle = std::move(other.m_Handle);
			m_Scene = std::move(other.m_Scene);
		}

		template<typename T>
		bool HasComponent()
		{
			return m_Scene->m_Registry.all_of<T>(m_Handle);
		}

		template<typename T>
		bool HasNativeScript()
		{
			return m_Scene->m_Registry.all_of<T>(m_Handle);
		}

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			PG_CORE_ASSERT(!HasComponent<T>(), "Entity already has component!");
			T& component = m_Scene->m_Registry.emplace<T>(m_Handle, std::forward<Args>(args)...);
			//m_Scene->OnComponentAdded<T>(*this, component);
			return component;
		}
		template<typename T, typename... Args>
		T& AddOrReplaceComponent(Args&&... args)
		{
			T& component = m_Scene->m_Registry.emplace_or_replace<T>(m_Handle, std::forward<Args>(args)...);
			//m_Scene->OnComponentAdded<T>(*this, component);
			return component;
		}

		// Removed OnComponentAdded event
		template<typename T, typename... Args>
		T& AddCustomComponent(Args&&... args)
		{
			PG_CORE_ASSERT(!HasComponent<T>(), "Entity already has component!");
			T& component = m_Scene->m_Registry.emplace<T>(m_Handle, std::forward<Args>(args)...);
			return component;
		}

		template<typename T>
		T& GetComponent()
		{
			PG_CORE_ASSERT(HasComponent<T>(), "Entity does not have component! {0}");
			return m_Scene->m_Registry.get<T>(m_Handle);
		}

		template<typename T>
		void RemoveComponent()
		{
			PG_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
			m_Scene->m_Registry.remove<T>(m_Handle);
		}

		inline bool IsValid()
		{
			return m_Scene != nullptr && m_Handle != entt::null;
		}

		inline Entity operator=(const Entity& other)
		{
			m_Handle = other.m_Handle;
			m_Scene = other.m_Scene;
			return *this;
		}

		inline Entity& operator=(Entity&& other) noexcept
		{
			m_Handle = std::move(other.m_Handle);
			m_Scene = std::move(other.m_Scene);
			return *this;
		}

		//inline TransformComponent& Transform() { return m_Scene->m_Registry.get<TransformComponent>(m_EntityHandle); }

		operator bool() const { return m_Handle != entt::null; }
		operator uint32_t() const { return (uint32_t)m_Handle; }
		operator entt::entity() const { return m_Handle; }

		//inline UUID GetUUID() { return GetComponent<IDComponent>().ID; }
		//inline const std::string& GetName() { return GetComponent<TagComponent>().Tag; }

		bool operator==(const Entity& other) const { return m_Handle == other.m_Handle && m_Scene == other.m_Scene; }
		bool operator!=(const Entity& other) const { return (*this == other) == false; }

	private:
		Scene* m_Scene{ nullptr };
		entt::entity m_Handle{ entt::null };
	};

}