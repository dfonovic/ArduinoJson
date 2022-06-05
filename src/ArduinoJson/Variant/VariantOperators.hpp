// ArduinoJson - https://arduinojson.org
// Copyright © 2014-2022, Benoit BLANCHON
// MIT License

#pragma once

#include <ArduinoJson/Misc/Visitable.hpp>
#include <ArduinoJson/Numbers/arithmeticCompare.hpp>
#include <ArduinoJson/Polyfills/attributes.hpp>
#include <ArduinoJson/Polyfills/type_traits.hpp>
#include <ArduinoJson/Variant/VariantTag.hpp>

namespace ARDUINOJSON_NAMESPACE {

class VariantConstRef;

template <typename T2>
CompareResult compare(VariantConstRef lhs,
                      const T2 &rhs);  // VariantCompare.cpp

template <typename TVariant>
struct VariantOperators {
  // Returns the default value if the VariantRef is unbound or incompatible
  //
  // int operator|(JsonVariant, int)
  // float operator|(JsonVariant, float)
  // bool operator|(JsonVariant, bool)
  template <typename T>
  friend
      typename enable_if<!IsVariant<T>::value && !is_array<T>::value, T>::type
      operator|(const TVariant &variant, const T &defaultValue) {
    if (variant.template is<T>())
      return variant.template as<T>();
    else
      return defaultValue;
  }
  //
  // const char* operator|(JsonVariant, const char*)
  friend const char *operator|(const TVariant &variant,
                               const char *defaultValue) {
    if (variant.template is<const char *>())
      return variant.template as<const char *>();
    else
      return defaultValue;
  }
  //
  // JsonVariant operator|(JsonVariant, JsonVariant)
  template <typename T>
  friend typename enable_if<IsVariant<T>::value, typename T::variant_type>::type
  operator|(const TVariant &variant, T defaultValue) {
    if (variant)
      return variant;
    else
      return defaultValue;
  }
};
}  // namespace ARDUINOJSON_NAMESPACE
