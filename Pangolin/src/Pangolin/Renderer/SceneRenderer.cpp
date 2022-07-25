#include "pgpch.h"
#include "SceneRenderer.h"

#include "Pangolin/Entities/Components.h"
#include "Pangolin/Entities/Entity.h"

#include "Pangolin/Renderer/RenderManager.h"

#include "Pangolin/Renderer/Camera.h"

namespace PG {

	SceneRenderer::SceneRenderer(Scene* scene)
		: m_Context(scene)
	{
	}

	SceneRenderer::~SceneRenderer()
	{
	}

	void SceneRenderer::OnUpdateRuntime(Timestep& ts)
	{
	}

	void SceneRenderer::OnRenderEditor(const Camera& camera)
	{
		// 2D Rendering
		auto& renderer2d = RenderManager::GetRenderer2D();
		// Quads
		{
			renderer2d->BeginScene(camera);

			auto& view = m_Context->GetAllEntitiesWith<TransformComponent, SpriteRendererComponent>();
			for (auto entity : view)
			{
				auto& [transformComponent, spriteComponent] = view.get<TransformComponent, SpriteRendererComponent>(entity);

				renderer2d->DrawSprite(transformComponent, spriteComponent, (int)entity);
			}
			renderer2d->EndScene();
		}
	}

	SceneID SceneRenderer::GetRenderedTexture() const
	{
		auto& renderer2d = RenderManager::GetRenderer2D(); // FIXME: Not ideal
		return renderer2d->GetTexture();
	}

}

