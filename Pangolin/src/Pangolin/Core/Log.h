#pragma once

#include "Common.h"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

namespace PG {

	class Log {
	public:
		static void Init();
		static void Shutdown();

		inline static Ref<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static Ref<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	private:
		static Ref<spdlog::logger> s_CoreLogger;
		static Ref<spdlog::logger> s_ClientLogger;
	};
}