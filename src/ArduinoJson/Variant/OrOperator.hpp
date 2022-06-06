// ArduinoJson - https://arduinojson.org
// Copyright © 2014-2022, Benoit BLANCHON
// MIT License

#pragma once

#include <ArduinoJson/Namespace.hpp>
#include <ArduinoJson/Polyfills/type_traits.hpp>
#include <ArduinoJson/Variant/VariantTag.hpp>

namespace ARDUINOJSON_NAMESPACE {

// Returns the default value if the VariantRef is unbound or incompatible
//
// int operator|(JsonVariant, int)
// float operator|(JsonVariant, float)
// bool operator|(JsonVariant, bool)
template <typename T>
inline typename enable_if<!IsVariant<T>::value &&
                              !ConverterNeedsWriteableRef<T>::value &&
                              !is_array<T>::value,
                          T>::type
operator|(VariantConstRef variant, const T &defaultValue) {
  if (variant.is<T>())
    return variant.as<T>();
  else
    return defaultValue;
}

template <typename T>
inline typename enable_if<!IsVariant<T>::value &&
                              ConverterNeedsWriteableRef<T>::value &&
                              !is_array<T>::value,
                          T>::type
operator|(VariantRef variant, const T &defaultValue) {
  if (variant.is<T>())
    return variant.as<T>();
  else
    return defaultValue;
}

//
// const char* operator|(JsonVariant, const char*)
inline const char *operator|(VariantConstRef variant,
                             const char *defaultValue) {
  if (variant.is<const char *>())
    return variant.as<const char *>();
  else
    return defaultValue;
}
//
// JsonVariant operator|(JsonVariant, ElementProxy)
template <typename TArray>
inline VariantRef operator|(VariantRef variant,
                            const ElementProxy<TArray> &defaultValue) {
  if (variant)
    return variant;
  else
    return defaultValue;
}
//
// JsonVariant operator|(JsonVariant, MemberProxy)
template <typename TObject, typename TStringRef>
inline VariantRef operator|(
    VariantRef variant, const MemberProxy<TObject, TStringRef> &defaultValue) {
  if (variant)
    return variant;
  else
    return defaultValue;
}

}  // namespace ARDUINOJSON_NAMESPACE
