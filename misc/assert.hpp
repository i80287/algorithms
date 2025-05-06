#include <cstdint>
#include <stdexcept>
#include <string>

#include "config_macros.hpp"

namespace misc {

namespace detail {

ATTRIBUTE_NONNULL_ALL_ARGS
[[nodiscard]] std::runtime_error make_exception(const char* const message,
                                                const char* const function_name) {
    const std::string_view message_sv(message);
    const std::string_view function_name_sv(function_name);
    std::string message_str(message_sv.size() + function_name_sv.size(), '\0');
    std::char_traits<char>::copy(message_str.data(), message_sv.data(), message_sv.size());
    std::char_traits<char>::copy(message_str.data() + message_sv.size(), function_name_sv.data(),
                                 function_name_sv.size());
    return std::runtime_error{message_str};
}

ATTRIBUTE_NONNULL_ALL_ARGS
ATTRIBUTE_NORETURN
inline void throw_runtime_error_impl(const char* const message, const char* const function_name) {
    throw misc::detail::make_exception(message, function_name);
}

ATTRIBUTE_NONNULL_ALL_ARGS
ATTRIBUTE_ALWAYS_INLINE
inline void throw_if_impl(const bool expression,
                          const char* const message,
                          const char* const function_name) {
    if (unlikely(expression)) {
        misc::detail::throw_runtime_error_impl(message, function_name);
    }
}

ATTRIBUTE_NONNULL_ALL_ARGS
ATTRIBUTE_ALWAYS_INLINE
inline void throw_if_not_impl(const bool expression,
                              const char* const message,
                              const char* const function_name) {
    if (unlikely(!expression)) {
        misc::detail::throw_runtime_error_impl(message, function_name);
    }
}

#define PP_EVAL_IMPL(arg)            arg
#define STRINGIFY_IMPL_2(expression) #expression
#define STRINGIFY_IMPL_1(expression) STRINGIFY_IMPL_2(expression)
#define THROW_IF_MESSAGE_IMPL(expression, bool_value)                 \
    "Expression \"" #expression "\" evaluated to " #bool_value        \
    " at " PP_EVAL_IMPL(__FILE__) ":" STRINGIFY_IMPL_1(__LINE__) ":", \
        CONFIG_CURRENT_FUNCTION_NAME

}  // namespace detail

#define THROW_IF_NOT(expression) \
    misc::detail::throw_if_not_impl(expression, THROW_IF_MESSAGE_IMPL(expression, "false"))

#define THROW_IF(expression) \
    misc::detail::throw_if_impl(expression, THROW_IF_MESSAGE_IMPL(expression, "true"))

}  // namespace misc
