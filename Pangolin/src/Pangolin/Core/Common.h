#pragma once

#include <memory>

namespace PG {

	template<typename T>
	using Scope = std::unique_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using WeakRef = std::weak_ptr<T>;

}

#define BIT(x) (1 << x)

#ifdef PG_DEBUG
	#if defined(PG_PLATFORM_WINDOWS)
		#define PG_DEBUGBREAK() __debugbreak()
	#elif defined(PG_PLATFORM_LINUX)
		#include <signal.h>
		#define PG_DEBUGBREAK() raise(SIGTRAP)
	#else
		#error "Platform doesn't support debugbreak yet!"
	#endif

	#define PG_ENABLE_ASSERTS
#else
	#define PG_DEBUGBREAK()
#endif

#define PG_BIND_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); } 

#include "Log.h"

#ifdef PG_DEBUG
//Client log macros
#define PG_TRACE(...)		::PG::Log::GetClientLogger()->trace(__VA_ARGS__)
#define PG_INFO(...)		::PG::Log::GetClientLogger()->info(__VA_ARGS__)
#define PG_WARN(...)		::PG::Log::GetClientLogger()->warn(__VA_ARGS__)
#define PG_ERROR(...)		::PG::Log::GetClientLogger()->error(__VA_ARGS__)
#define PG_FATAL(...)		::PG::Log::GetClientLogger()->critical(__VA_ARGS__)
//Core log macros
#define PG_CORE_TRACE(...)	::PG::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define PG_CORE_INFO(...)	::PG::Log::GetCoreLogger()->info(__VA_ARGS__)
#define PG_CORE_WARN(...)	::PG::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define PG_CORE_ERROR(...)	::PG::Log::GetCoreLogger()->error(__VA_ARGS__)
#define PG_CORE_FATAL(...)	::PG::Log::GetCoreLogger()->critical(__VA_ARGS__)
#elif PG_RELEASE
//Client log macros
#define PG_ERROR(...)		
#define PG_WARN(...)		
#define PG_TRACE(...)		
#define PG_INFO(...)			
#define PG_FATAL(...)		
//Core log macros
#define PG_CORE_ERROR(...)
#define PG_CORE_WARN(...)
#define PG_CORE_TRACE(...)
#define PG_CORE_INFO(...)
#define PG_CORE_FATAL(...)
#endif

#include "Pangolin/Core/Assert.h"