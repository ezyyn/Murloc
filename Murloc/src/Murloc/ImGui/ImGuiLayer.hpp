#pragma once

#include "Murloc/Core/Layer.hpp"

namespace Murloc {

	class ImGuiLayer : public Layer {
	public:
		void Begin();
		void End();

		void OnAttach() override;
		void OnDetach() override;
		void OnImGuiRender() {};

	private:
		void FrameRender();
	};
}