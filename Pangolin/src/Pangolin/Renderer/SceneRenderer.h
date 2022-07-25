#pragma once

#include "Pangolin/Core/Time.h"

namespace PG {

	class Camera;
	class Scene;

	struct SceneSettings {

	};

	class SceneRenderer
	{
	public:
		SceneRenderer(Scene* scene);
		~SceneRenderer();

		void OnUpdateRuntime(Timestep& ts);
		void OnRenderEditor(const Camera& camera);
	private:
		void* GetRenderedTexture() const;

		Scene* m_Context;

		friend class Scene;
	};
}