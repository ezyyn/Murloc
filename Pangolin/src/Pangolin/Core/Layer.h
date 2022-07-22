#pragma once

#include "Pangolin/Event/Event.h"
#include "Pangolin/Core/Time.h"

namespace PG {

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