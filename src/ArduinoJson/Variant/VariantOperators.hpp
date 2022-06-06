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

template <typename TVariant>
struct VariantOperators {};
}  // namespace ARDUINOJSON_NAMESPACE
