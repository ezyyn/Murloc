#pragma once

#include "Murloc/Event/Event.hpp"
#include "Murloc/Core/KeyCodes.hpp"

namespace Murloc {

	class KeyEvent : public Event 
	{
	public:
		KeyCode GetKeyCode() const{ return m_KeyCode; }

		EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
	protected:
		KeyEvent(const KeyCode keycode)
			: m_KeyCode(keycode) {}
	private:
		KeyCode m_KeyCode;
	};

	class KeyPressedEvent : public KeyEvent 
	{
	public:
		KeyPressedEvent(KeyCode code, uint16_t repeatCount)  
			: KeyEvent(code), m_RepeatCount(repeatCount) {}

		uint16_t GetRepeatCount() const { return m_RepeatCount; }

		EVENT_CLASS_TYPE(KeyPressed)
	private:
		uint16_t m_RepeatCount;
	};

	class KeyReleasedEvent : public KeyEvent
	{
	public:
		KeyReleasedEvent(KeyCode code)
			: KeyEvent(code) {}


		EVENT_CLASS_TYPE(KeyReleased)

	};

	class KeyTypedEvent : public KeyEvent
	{
	public:
		KeyTypedEvent(KeyCode code)
			: KeyEvent(code) {}

		EVENT_CLASS_TYPE(KeyTyped)

	};
}