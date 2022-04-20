#pragma once

#include "Log.hpp"
#include <memory>

namespace Murloc {

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

#ifdef MUR_DEBUG
	#if defined(MUR_PLATFORM_WINDOWS)
		#define MUR_DEBUGBREAK() __debugbreak()
	#elif defined(MUR_PLATFORM_LINUX)
		#include <signal.h>
		#define MUR_DEBUGBREAK() raise(SIGTRAP)
#else
#error "Platform doesn't support debugbreak yet!"
#endif
	#define MUR_ENABLE_ASSERTS
#else
#define MUR_DEBUGBREAK()
#endif

#define MUR_BIND_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); } 

#ifdef MUR_DEBUG
//Client log macros
#define MUR_ERROR(...)		::Murloc::Log::GetClientLogger()->error(__VA_ARGS__);
#define MUR_WARN(...)		::Murloc::Log::GetClientLogger()->warn(__VA_ARGS__);
#define MUR_TRACE(...)		::Murloc::Log::GetClientLogger()->trace(__VA_ARGS__);
#define MUR_INFO(...)		::Murloc::Log::GetClientLogger()->info(__VA_ARGS__);
#define MUR_INFO(...)		::Murloc::Log::GetClientLogger()->info(__VA_ARGS__);
#define MUR_FATAL(...)		::Murloc::Log::GetClientLogger()->critical(__VA_ARGS__);
//Core log macros
#define MUR_CORE_ERROR(...)	::Murloc::Log::GetCoreLogger()->error(__VA_ARGS__);
#define MUR_CORE_WARN(...)	::Murloc::Log::GetCoreLogger()->warn(__VA_ARGS__);
#define MUR_CORE_TRACE(...)	::Murloc::Log::GetCoreLogger()->trace(__VA_ARGS__);
#define MUR_CORE_INFO(...)	::Murloc::Log::GetCoreLogger()->info(__VA_ARGS__);
#define MUR_CORE_FATAL(...)	::Murloc::Log::GetCoreLogger()->critical(__VA_ARGS__);
#elif MUR_RELEASE
//Client log macros
#define MUR_ERROR(...)		
#define MUR_WARN(...)		
#define MUR_TRACE(...)		
#define MUR_INFO(...)		
#define MUR_INFO(...)		
#define MUR_FATAL(...)		
//Core log macros
#define MUR_CORE_ERROR(...)
#define MUR_CORE_WARN(...)
#define MUR_CORE_TRACE(...)
#define MUR_CORE_INFO(...)
#define MUR_CORE_FATAL(...)
#endif

