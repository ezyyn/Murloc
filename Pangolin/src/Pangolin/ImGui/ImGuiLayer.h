#pragma once

#include "Pangolin/Core/Layer.h"

#include "Pangolin/Event/ApplicationEvent.h"

namespace PG {

	class ImGuiLayer : public Layer {
	public:
		void Begin();
		void End();

		void OnAttach() override;
		void OnDetach() override;

		void OnImGuiRender() {};

		void OnEvent(Event& e) override;

	private:
		bool OnWindowResize(WindowResizeEvent& e);

		void FrameRender();
	};
}