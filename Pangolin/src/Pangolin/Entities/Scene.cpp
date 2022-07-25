#include "pgpch.h"

#include "Scene.h"

#include "Components.h"
#include "Entity.h"

#include "Pangolin/Renderer/RenderManager.h"

namespace PG {
	
	Scene::Scene()
	{
	/*	auto& entity = CreateEntity("Test");
        
		auto& src = entity.AddComponent<SpriteRendererComponent>();
		src.Color = { 1.0f,1.0f, 1.0f, 1.0f };
        
		entity.GetComponent<TransformComponent>().Rotation.x = 180.0;*/

		m_Renderer = CreateRef<SceneRenderer>(this);
	}
    
	Scene::~Scene()
	{
	}
    
	void Scene::OnUpdateRuntime(Timestep& ts)
	{
		//m_Renderer->OnUpdateRuntime(camera);
	}
    
	void Scene::OnUpdateEditor(Timestep& ts, const EditorCamera& camera)
	{
		// Rendering
		m_Renderer->OnRenderEditor(camera);
	}
    
	Entity Scene::CreateEntity(const std::string& name)
	{
		Entity& newEntity = Entity(m_Registry.create(), this);
        
		newEntity.AddComponent<TransformComponent>();
		newEntity.AddComponent<TagComponent>().Tag = name;
        
		return newEntity;
	}
    
	void Scene::DestroyEntity(const Entity& entity)
	{
		m_Registry.destroy(entity);
	}

	SceneID Scene::GetRenderedScene() const
	{
		return m_Renderer->GetRenderedTexture();
	}

}