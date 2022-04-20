#pragma once

#include "Event.hpp"

namespace Murloc {

	class KeyEvent : public Event 
	{
	public:

		EventType GetEventType() const override
		{
			throw std::logic_error("The method or operation is not implemented.");
		}


		const char* GetName() const override
		{
			throw std::logic_error("The method or operation is not implemented.");
		}


		int GetCategoryFlags() const override
		{
			throw std::logic_error("The method or operation is not implemented.");
		}

	};

}