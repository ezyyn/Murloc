#pragma once

#include <chrono>

#include "Murloc/Core/Common.hpp"

namespace Murloc {

	class ScopedTimer {
	public:
		ScopedTimer(const char* name) {
			m_Start = std::chrono::high_resolution_clock::now();
			MUR_CORE_ASSERT(sizeof(name) < 30 * 4);
			strcpy(m_Name, name);
		}
		~ScopedTimer() {

			m_End = std::chrono::high_resolution_clock::now();

			std::chrono::duration<float> duration = m_End - m_Start;
			MUR_CORE_WARN("{0} took {1} ms", m_Name, duration.count() * 1000.0f);
		}
	private:
		std::chrono::time_point<std::chrono::steady_clock> m_Start, m_End;
		char m_Name[30];
	};

}