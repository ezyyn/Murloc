#pragma once

#include "Murloc/Core/Layer.hpp"

namespace Murloc {

	class ImGuiLayer : public Layer {
	public:

		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate(Timestep& ts) override;
		void OnImGuiRender() override;
		void OnEvent(Event& e) override;

	};
}