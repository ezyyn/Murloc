#pragma once

#include "Vulkan/GraphicsPipeline.h"
#include "Vulkan/RenderPass.h"

namespace Murloc {
    
	class CommandQueue;
	struct RenderData;

    // Specifications for this renderer
    struct RendererInfo 
    {
        Ref<GraphicsPipeline> Pipeline;
        Ref<Shader> Shader;
		Ref<RenderPass> RenderPass;
    };

    class BatchRenderer 
    {
    public:
        BatchRenderer(const RendererInfo& info = {});
        ~BatchRenderer();
    private:
        void Init();

		void BeginScene();
		void EndScene();

        void Shutdown();

        RenderData* m_Data{ nullptr };

        RendererInfo m_RendererInfo{ nullptr };
    };
    
}