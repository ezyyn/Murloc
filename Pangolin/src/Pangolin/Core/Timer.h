#pragma once

#include <chrono>

#include "Pangolin/Core/Common.h"

#include <string>

namespace PG {

	class ScopedTimer {
	public:
		ScopedTimer(const std::string& name, bool output = true)
			: m_Name(name), m_Output(output)
		{
			m_Start = std::chrono::high_resolution_clock::now();
		}

		float Stop() {
			m_End = std::chrono::high_resolution_clock::now();

			std::chrono::duration<float> duration = m_End - m_Start;
			return duration.count() * 1000.0f;
		}
		~ScopedTimer() {
			if (m_Output == false)
				return;
			m_End = std::chrono::high_resolution_clock::now();

			std::chrono::duration<float> duration = m_End - m_Start;
			PG_CORE_WARN("{0} took {1} ms", m_Name, duration.count() * 1000.0f);
		}
	private:
		std::chrono::time_point<std::chrono::steady_clock> m_Start, m_End;
		std::string m_Name;
		bool m_Output;
	};

}