#pragma once

#include "Murloc/Event/Event.hpp"
#include "Murloc/Core/Time.hpp"

namespace Murloc {

	class Layer {
	public:
		Layer() {}
		virtual ~Layer() {}

		virtual void OnAttach() = 0;
		virtual void OnDetach() = 0;
		virtual void OnUpdate(Timestep& ts) = 0;
		virtual void OnImGuiRender() {};
		virtual void OnEvent(Event& e) = 0;
	};

}