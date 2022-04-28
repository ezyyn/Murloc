#pragma once

#include "Murloc/Event/Event.hpp"
#include "Murloc/Core/Time.hpp"

namespace Murloc {

	class Layer {
	public:
		Layer() {}
		virtual ~Layer() {}

		virtual void OnAttach(){};
		virtual void OnDetach() {};
		virtual void OnUpdate(Timestep& ts) {};
		virtual void OnImGuiRender() {};
		virtual void OnEvent(Event& e) {};
	};

}