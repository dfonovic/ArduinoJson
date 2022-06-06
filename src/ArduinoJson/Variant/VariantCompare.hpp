// ArduinoJson - https://arduinojson.org
// Copyright © 2014-2022, Benoit BLANCHON
// MIT License

#pragma once

#include <ArduinoJson/Configuration.hpp>
#include <ArduinoJson/Misc/Visitable.hpp>
#include <ArduinoJson/Numbers/arithmeticCompare.hpp>
#include <ArduinoJson/Polyfills/type_traits.hpp>
#include <ArduinoJson/Strings/StringAdapters.hpp>
#include <ArduinoJson/Variant/Visitor.hpp>

namespace ARDUINOJSON_NAMESPACE {

class CollectionData;

struct ComparerBase : Visitor<CompareResult> {};

template <typename T, typename Enable = void>
struct Comparer;

template <typename T>
struct Comparer<T, typename enable_if<IsString<T>::value>::type>
    : ComparerBase {
  T rhs;  // TODO: store adapted string?

  explicit Comparer(T value) : rhs(value) {}

  CompareResult visitString(const char *lhs, size_t n) {
    int i = stringCompare(adaptString(rhs), adaptString(lhs, n));
    if (i < 0)
      return COMPARE_RESULT_GREATER;
    else if (i > 0)
      return COMPARE_RESULT_LESS;
    else
      return COMPARE_RESULT_EQUAL;
  }

  CompareResult visitNull() {
    if (adaptString(rhs).isNull())
      return COMPARE_RESULT_EQUAL;
    else
      return COMPARE_RESULT_DIFFER;
  }
};

template <typename T>
struct Comparer<T, typename enable_if<is_integral<T>::value ||
                                      is_floating_point<T>::value>::type>
    : ComparerBase {
  T rhs;

  explicit Comparer(T value) : rhs(value) {}

  CompareResult visitFloat(Float lhs) {
    return arithmeticCompare(lhs, rhs);
  }

  CompareResult visitSignedInteger(Integer lhs) {
    return arithmeticCompare(lhs, rhs);
  }

  CompareResult visitUnsignedInteger(UInt lhs) {
    return arithmeticCompare(lhs, rhs);
  }

  CompareResult visitBoolean(bool lhs) {
    return visitUnsignedInteger(static_cast<UInt>(lhs));
  }
};

struct NullComparer : ComparerBase {
  CompareResult visitNull() {
    return COMPARE_RESULT_EQUAL;
  }
};

#if ARDUINOJSON_HAS_NULLPTR
template <>
struct Comparer<decltype(nullptr), void> : NullComparer {
  explicit Comparer(decltype(nullptr)) : NullComparer() {}
};
#endif

struct ArrayComparer : ComparerBase {
  const CollectionData *_rhs;

  explicit ArrayComparer(const CollectionData &rhs) : _rhs(&rhs) {}

  CompareResult visitArray(const CollectionData &lhs);
};

struct ObjectComparer : ComparerBase {
  const CollectionData *_rhs;

  explicit ObjectComparer(const CollectionData &rhs) : _rhs(&rhs) {}

  CompareResult visitObject(const CollectionData &lhs);
};

struct RawComparer : ComparerBase {
  const char *_rhsData;
  size_t _rhsSize;

  explicit RawComparer(const char *rhsData, size_t rhsSize)
      : _rhsData(rhsData), _rhsSize(rhsSize) {}

  CompareResult visitRawJson(const char *lhsData, size_t lhsSize) {
    size_t size = _rhsSize < lhsSize ? _rhsSize : lhsSize;
    int n = memcmp(lhsData, _rhsData, size);
    if (n < 0)
      return COMPARE_RESULT_LESS;
    else if (n > 0)
      return COMPARE_RESULT_GREATER;
    else
      return COMPARE_RESULT_EQUAL;
  }
};

struct VariantComparer : ComparerBase {
  VariantConstRef rhs;

  explicit VariantComparer(VariantConstRef value) : rhs(value) {}

  CompareResult visitArray(const CollectionData &lhs) {
    ArrayComparer comparer(lhs);
    return accept(comparer);
  }

  CompareResult visitObject(const CollectionData &lhs) {
    ObjectComparer comparer(lhs);
    return accept(comparer);
  }

  CompareResult visitFloat(Float lhs) {
    Comparer<Float> comparer(lhs);
    return accept(comparer);
  }

  CompareResult visitString(const char *lhs, size_t) {
    Comparer<const char *> comparer(lhs);
    return accept(comparer);
  }

  CompareResult visitRawJson(const char *lhsData, size_t lhsSize) {
    RawComparer comparer(lhsData, lhsSize);
    return accept(comparer);
  }

  CompareResult visitSignedInteger(Integer lhs) {
    Comparer<Integer> comparer(lhs);
    return accept(comparer);
  }

  CompareResult visitUnsignedInteger(UInt lhs) {
    Comparer<UInt> comparer(lhs);
    return accept(comparer);
  }

  CompareResult visitBoolean(bool lhs) {
    Comparer<bool> comparer(lhs);
    return accept(comparer);
  }

  CompareResult visitNull() {
    NullComparer comparer;
    return accept(comparer);
  }

 private:
  template <typename TComparer>
  CompareResult accept(TComparer &comparer) {
    CompareResult reversedResult = rhs.accept(comparer);
    switch (reversedResult) {
      case COMPARE_RESULT_GREATER:
        return COMPARE_RESULT_LESS;
      case COMPARE_RESULT_LESS:
        return COMPARE_RESULT_GREATER;
      default:
        return reversedResult;
    }
  }
};

template <typename T>
struct Comparer<T, typename enable_if<IsVisitable<T>::value>::type>
    : VariantComparer {
  explicit Comparer(const T &value)
      : VariantComparer(value.operator VariantConstRef()) {}
};

template <typename T2>
CompareResult compare(VariantConstRef lhs, const T2 &rhs) {
  Comparer<T2> comparer(rhs);
  return lhs.accept(comparer);
}

// value == VariantConstRef
template <typename T>
inline bool operator==(T *lhs, VariantConstRef rhs) {
  return compare(rhs, lhs) == COMPARE_RESULT_EQUAL;
}
template <typename T>
inline bool operator==(const T &lhs, VariantConstRef rhs) {
  return compare(rhs, lhs) == COMPARE_RESULT_EQUAL;
}

// VariantConstRef == value
template <typename T>
inline bool operator==(VariantConstRef lhs, T *rhs) {
  return compare(lhs, rhs) == COMPARE_RESULT_EQUAL;
}
template <typename T>
inline typename enable_if<!IsVisitable<T>::value, bool>::type operator==(
    VariantConstRef lhs, const T &rhs) {
  return compare(lhs, rhs) == COMPARE_RESULT_EQUAL;
}

// value != VariantConstRef
template <typename T>
inline bool operator!=(T *lhs, VariantConstRef rhs) {
  return compare(rhs, lhs) != COMPARE_RESULT_EQUAL;
}
template <typename T>
inline bool operator!=(const T &lhs, VariantConstRef rhs) {
  return compare(rhs, lhs) != COMPARE_RESULT_EQUAL;
}

// VariantConstRef != value
template <typename T>
inline bool operator!=(VariantConstRef lhs, T *rhs) {
  return compare(lhs, rhs) != COMPARE_RESULT_EQUAL;
}
template <typename T>
inline typename enable_if<!IsVisitable<T>::value, bool>::type operator!=(
    VariantConstRef lhs, const T &rhs) {
  return compare(lhs, rhs) != COMPARE_RESULT_EQUAL;
}

// value < VariantConstRef
template <typename T>
inline bool operator<(T *lhs, VariantConstRef rhs) {
  return compare(rhs, lhs) == COMPARE_RESULT_GREATER;
}
template <typename T>
inline bool operator<(const T &lhs, VariantConstRef rhs) {
  return compare(rhs, lhs) == COMPARE_RESULT_GREATER;
}

// VariantConstRef < value
template <typename T>
inline bool operator<(VariantConstRef lhs, T *rhs) {
  return compare(lhs, rhs) == COMPARE_RESULT_LESS;
}
template <typename T>
inline typename enable_if<!IsVisitable<T>::value, bool>::type operator<(
    VariantConstRef lhs, const T &rhs) {
  return compare(lhs, rhs) == COMPARE_RESULT_LESS;
}

// value <= VariantConstRef
template <typename T>
inline bool operator<=(T *lhs, VariantConstRef rhs) {
  return (compare(rhs, lhs) & COMPARE_RESULT_GREATER_OR_EQUAL) != 0;
}
template <typename T>
inline bool operator<=(const T &lhs, VariantConstRef rhs) {
  return (compare(rhs, lhs) & COMPARE_RESULT_GREATER_OR_EQUAL) != 0;
}

// VariantConstRef <= value
template <typename T>
inline bool operator<=(VariantConstRef lhs, T *rhs) {
  return (compare(lhs, rhs) & COMPARE_RESULT_LESS_OR_EQUAL) != 0;
}
template <typename T>
inline typename enable_if<!IsVisitable<T>::value, bool>::type operator<=(
    VariantConstRef lhs, const T &rhs) {
  return (compare(lhs, rhs) & COMPARE_RESULT_LESS_OR_EQUAL) != 0;
}

// value > VariantConstRef
template <typename T>
inline bool operator>(T *lhs, VariantConstRef rhs) {
  return compare(rhs, lhs) == COMPARE_RESULT_LESS;
}
template <typename T>
inline bool operator>(const T &lhs, VariantConstRef rhs) {
  return compare(rhs, lhs) == COMPARE_RESULT_LESS;
}

// VariantConstRef > value
template <typename T>
inline bool operator>(VariantConstRef lhs, T *rhs) {
  return compare(lhs, rhs) == COMPARE_RESULT_GREATER;
}
template <typename T>
inline typename enable_if<!IsVisitable<T>::value, bool>::type operator>(
    VariantConstRef lhs, const T &rhs) {
  return compare(lhs, rhs) == COMPARE_RESULT_GREATER;
}

// value >= VariantConstRef
template <typename T>
inline bool operator>=(T *lhs, VariantConstRef rhs) {
  return (compare(rhs, lhs) & COMPARE_RESULT_LESS_OR_EQUAL) != 0;
}
template <typename T>
inline bool operator>=(const T &lhs, VariantConstRef rhs) {
  return (compare(rhs, lhs) & COMPARE_RESULT_LESS_OR_EQUAL) != 0;
}

// VariantConstRef >= value
template <typename T>
inline bool operator>=(VariantConstRef lhs, T *rhs) {
  return (compare(lhs, rhs) & COMPARE_RESULT_GREATER_OR_EQUAL) != 0;
}
template <typename T>
inline typename enable_if<!IsVisitable<T>::value, bool>::type operator>=(
    VariantConstRef lhs, const T &rhs) {
  return (compare(lhs, rhs) & COMPARE_RESULT_GREATER_OR_EQUAL) != 0;
}

inline CompareResult ArrayComparer::visitArray(const CollectionData &lhs) {
  const CollectionData &rhs = *_rhs;

  for (const VariantSlot *slot1 = lhs.head(), *slot2 = rhs.head();
       slot1 || slot2; slot1 = slot1->next(), slot2 = slot2->next()) {
    VariantConstRef val1(reinterpret_cast<const VariantData *>(slot1));
    VariantConstRef val2(reinterpret_cast<const VariantData *>(slot2));
    if (val1 != val2)
      return COMPARE_RESULT_DIFFER;
  }

  return COMPARE_RESULT_EQUAL;
}

inline CompareResult ObjectComparer::visitObject(const CollectionData &lhs) {
  const CollectionData &rhs = *_rhs;

  size_t count = 0;
  for (const VariantSlot *slot = lhs.head(); slot; slot = slot->next()) {
    VariantConstRef val1(reinterpret_cast<const VariantData *>(slot));
    VariantConstRef val2(rhs.getMember(adaptString(slot->key())));
    if (val1 != val2)
      return COMPARE_RESULT_DIFFER;
    count++;
  }
  return count == rhs.size() ? COMPARE_RESULT_EQUAL : COMPARE_RESULT_DIFFER;
}

// TODO: MOVE

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
// JsonVariant operator|(JsonVariant, MemberProxy/ElementProxy)
template <typename T>
inline typename enable_if<IsVariant<T>::value, VariantRef>::type operator|(
    VariantRef variant, T defaultValue) {
  if (variant)
    return variant;
  else
    return defaultValue;
}

}  // namespace ARDUINOJSON_NAMESPACE
