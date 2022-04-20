#pragma once

#include "Common.hpp"
#include <filesystem>

#ifdef MUR_ENABLE_ASSERTS

// Alteratively we could use the same "default" message for both "WITH_MSG" and "NO_MSG" and
// provide support for custom formatting by concatenating the formatting string instead of having the format inside the default message
#define MUR_INTERNAL_ASSERT_IMPL(type, check, msg, ...) { if(!(check)) { MUR##type##ERROR(msg, __VA_ARGS__); MUR_DEBUGBREAK(); } }
#define MUR_INTERNAL_ASSERT_WITH_MSG(type, check, ...) MUR_INTERNAL_ASSERT_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
#define MUR_INTERNAL_ASSERT_NO_MSG(type, check) MUR_INTERNAL_ASSERT_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", MUR_STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)

#define MUR_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
#define MUR_INTERNAL_ASSERT_GET_MACRO(...) MUR_EXPAND_MACRO( MUR_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, MUR_INTERNAL_ASSERT_WITH_MSG, MUR_INTERNAL_ASSERT_NO_MSG) )

// Currently accepts at least the condition and one additional parameter (the message) being optional
#define MUR_ASSERT(...) MUR_EXPAND_MACRO( MUR_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__) )
#define MUR_CORE_ASSERT(...) MUR_EXPAND_MACRO( MUR_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_CORE_, __VA_ARGS__) )
#else
#define MUR_ASSERT(...)
#define MUR_CORE_ASSERT(...)
#endif