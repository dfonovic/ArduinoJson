// ArduinoJson - https://arduinojson.org
// Copyright © 2014-2022, Benoit BLANCHON
// MIT License

#pragma once

#include <ArduinoJson/Array/ArrayRef.hpp>
#include <ArduinoJson/Object/ObjectRef.hpp>
#include <ArduinoJson/Variant/VariantAttorney.hpp>

namespace ARDUINOJSON_NAMESPACE {

template <typename TArray>
inline ArrayRef ArrayShortcuts<TArray>::createNestedArray() const {
  return add().template as<ArrayRef>();
}

template <typename TArray>
inline ObjectRef ArrayShortcuts<TArray>::createNestedObject() const {
  return add().template as<ObjectRef>();
}

template <typename TArray>
inline ElementProxy<TArray> ArrayShortcuts<TArray>::operator[](
    size_t index) const {
  return ElementProxy<TArray>(impl(), index);
}

template <typename TArray>
inline VariantRef ArrayShortcuts<TArray>::add() const {
  VariantData* data = VariantAttorney::getOrCreateData(impl());
  MemoryPool* pool = VariantAttorney::getPool(impl());
  return VariantRef(pool, variantAddElement(data, pool));
}

template <typename TArray>
template <typename T>
inline bool ArrayShortcuts<TArray>::add(const T& value) const {
  return add().set(value);
}

template <typename TArray>
template <typename T>
inline bool ArrayShortcuts<TArray>::add(T* value) const {
  return add().set(value);
}

}  // namespace ARDUINOJSON_NAMESPACE
