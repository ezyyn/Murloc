#pragma once

namespace PG {

	class Timestep {
	public:
		Timestep(float ts) : m_Timestep(ts) {}

		float GetMs() const { return m_Timestep * 1000; }
		float GetS() const { return m_Timestep; }

		float operator()() {
			return m_Timestep;
		}

	private:
		float m_Timestep;
	};
}