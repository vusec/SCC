#ifndef STRONGTYPEDEF_H
#define STRONGTYPEDEF_H

#include <cstdint>
#include <string>
#include <unordered_map>

/// A 'strong' typedef for an integer type.
template <class Subclass, typename UnderlyingT = std::size_t>
struct StrongTypedef {
  typedef StrongTypedef<Subclass, UnderlyingT> ThisType;
  typedef UnderlyingT Underlying;

  StrongTypedef() = default;
  ~StrongTypedef() = default;

  constexpr static Subclass fromInternalValue(Underlying value) {
    Subclass c;
    c.value = value;
    return c;
  }
  Underlying getInternalVal() const { return value; }
  bool operator==(const ThisType &other) const { return value == other.value; }
  bool operator!=(const ThisType &other) const { return value != other.value; }
  bool operator<(const ThisType &other) const { return value < other.value; }
  std::string toDebugStr() const { return std::to_string(value); }

private:
  Underlying value = 0;
};

#endif // STRONGTYPEDEF_H
