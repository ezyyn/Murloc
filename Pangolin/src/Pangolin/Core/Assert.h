#pragma once

#include "Common.h"
#include <filesystem>

#ifdef PG_ENABLE_ASSERTS

#define PG_EXPAND_MACRO(x) x
#define PG_STRINGIFY_MACRO(x) #x

// Alteratively we could use the same "default" message for both "WITH_MSG" and "NO_MSG" and
// provide support for custom formatting by concatenating the formatting string instead of having the format inside the default message
#define PG_INTERNAL_ASSERT_IMPL(type, check, msg, ...) { if(!(check)) { PG##type##ERROR(msg, __VA_ARGS__); PG_DEBUGBREAK(); } }
#define PG_INTERNAL_ASSERT_WITH_MSG(type, check, ...) PG_INTERNAL_ASSERT_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
#define PG_INTERNAL_ASSERT_NO_MSG(type, check) PG_INTERNAL_ASSERT_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", PG_STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)

#define PG_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
#define PG_INTERNAL_ASSERT_GET_MACRO(...) PG_EXPAND_MACRO( PG_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, PG_INTERNAL_ASSERT_WITH_MSG, PG_INTERNAL_ASSERT_NO_MSG) )

// Currently accepts at least the condition and one additional parameter (the message) being optional
#define PG_ASSERT(...) PG_EXPAND_MACRO( PG_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__) )
#define PG_CORE_ASSERT(...) PG_EXPAND_MACRO( PG_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_CORE_, __VA_ARGS__) )

#define PG_VK_ASSERT(x)	{ \
								VkResult __result = (x); \
								PG_CORE_ASSERT(__result  == VK_SUCCESS, __result); \
							}\

#else
#define PG_ASSERT(...)
#define PG_CORE_ASSERT(...)
#define PG_VK_ASSERT(x)
#endif