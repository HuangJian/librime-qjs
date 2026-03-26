#pragma once

#include <quickjs.h>
#include <memory>
#include <type_traits>

template <typename T_RIME_TYPE>
class JsWrapper {
public:
  // to satisfy clang-tidy -_-!
  using T_UNWRAP_TYPE = std::shared_ptr<T_RIME_TYPE>;
  inline static auto typeName = "Unknown";
  inline static JSClassID jsClassId = 0;
};

template <typename T_WRAPPER, typename = void>
struct has_properties_qjs_accessor : std::false_type {};
template <typename T_WRAPPER>
struct has_properties_qjs_accessor<
    T_WRAPPER,
    std::void_t<decltype(T_WRAPPER::propertiesQjs()), decltype(T_WRAPPER::propertiesSize())>>
    : std::true_type {};

template <typename T_WRAPPER, typename = void>
struct has_getters_qjs_accessor : std::false_type {};
template <typename T_WRAPPER>
struct has_getters_qjs_accessor<
    T_WRAPPER,
    std::void_t<decltype(T_WRAPPER::gettersQjs()), decltype(T_WRAPPER::gettersSize())>>
    : std::true_type {};

template <typename T_WRAPPER, typename = void>
struct has_functions_qjs_accessor : std::false_type {};
template <typename T_WRAPPER>
struct has_functions_qjs_accessor<
    T_WRAPPER,
    std::void_t<decltype(T_WRAPPER::functionsQjs()), decltype(T_WRAPPER::functionsSize())>>
    : std::true_type {};

#ifdef _ENABLE_JAVASCRIPTCORE
template <typename T_WRAPPER, typename = void>
struct has_properties_jsc_accessor : std::false_type {};
template <typename T_WRAPPER>
struct has_properties_jsc_accessor<
    T_WRAPPER,
    std::void_t<decltype(T_WRAPPER::binderPropertiesJsc()), decltype(T_WRAPPER::propertiesSize())>>
    : std::true_type {};

template <typename T_WRAPPER, typename = void>
struct has_getters_jsc_accessor : std::false_type {};
template <typename T_WRAPPER>
struct has_getters_jsc_accessor<
    T_WRAPPER,
    std::void_t<decltype(T_WRAPPER::binderGettersJsc()), decltype(T_WRAPPER::gettersSize())>>
    : std::true_type {};

template <typename T_WRAPPER, typename = void>
struct has_functions_jsc_accessor : std::false_type {};
template <typename T_WRAPPER>
struct has_functions_jsc_accessor<
    T_WRAPPER,
    std::void_t<decltype(T_WRAPPER::binderFunctionsJsc()), decltype(T_WRAPPER::functionsSize())>>
    : std::true_type {};
#endif
