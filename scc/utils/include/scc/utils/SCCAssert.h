#ifndef SCCASSERT_H
#define SCCASSERT_H

#include <optional>
#include <string>

/// Placeholder function.
/// Define a custom function/lambda in the assert scope to add custom
/// information.
inline std::string SCCAssertInfo() { return ""; }

namespace debugSupport {
template <typename T> inline std::string toString(T value) {
  return std::to_string(value);
}
} // namespace debugSupport

/// Asserts that the given condition is true. If not, aborts and prints the
/// given message.
#define SCCAssert(COND, MSG)                                                   \
  if (!(COND))                                                                 \
  SCCAssertImpl(#COND, {}, #COND, MSG, __FILE__, __PRETTY_FUNCTION__,          \
                __LINE__, SCCAssertInfo())

/// Aborts and prints the given message.
#define SCCError(MSG)                                                          \
  SCCAssertImpl({}, {}, "", MSG, __FILE__, __PRETTY_FUNCTION__, __LINE__,      \
                SCCAssertInfo())

/// Asserts that the two values are equal. Otherwise aborts.
#define SCCAssertEqual(LHS, RHS, MSG)                                          \
  if ((LHS) != (RHS))                                                          \
  SCCAssertImpl(debugSupport::toString(LHS), debugSupport::toString(RHS),      \
                #LHS " == " #RHS, MSG, __FILE__, __PRETTY_FUNCTION__,          \
                __LINE__, SCCAssertInfo())

/// Asserts that the two values are not equal.
#define SCCAssertNotEqual(LHS, RHS, MSG)                                       \
  if ((LHS) == (RHS))                                                          \
  SCCAssertImpl(debugSupport::toString(LHS), debugSupport::toString(RHS),      \
                #LHS " != " #RHS, MSG, __FILE__, __PRETTY_FUNCTION__,          \
                __LINE__, SCCAssertInfo())

/// Internal assert implementation.
///
/// @param lhsOrCond The condition that failed or the LHS of the comparison.
/// @param rhs The RHS of the comparison (or None if it's not a comparison).
/// @param checkStr The stringified version of the check.
/// @param msg The message to be printed.
/// @param file The file where the assert was written.
/// @param function The function within the assert was written.
/// @param line The source line where the assert was written.
/// @param extraInfo Additional string to be printed at the end.
[[noreturn]] void SCCAssertImpl(std::optional<std::string> lhsOrCond,
                                std::optional<std::string> rhs,
                                std::string checkStr, std::string msg,
                                std::string file, std::string function,
                                int line, std::string extraInfo);

#endif // SCCASSERT_H
