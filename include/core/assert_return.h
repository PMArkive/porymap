#pragma once

#include <QtAssert>

// A series of wrappers around Qt assert macros,
// with the addition that each will return if the test fails.
// This makes it easy to raise asserts in situations where you
// may want to do an early return even if debug is disabled.

#define Q_ASSERT_RETURN(test, ...) \
do { \
    Q_ASSERT(test); \
    if (!test) return __VA_ARGS__; \
} while (false);

#define Q_ASSERT_RETURN_X(test, where, what, ...) \
do { \
    Q_ASSERT_X(test, where, what); \
    if (!test) return __VA_ARGS__; \
} while (false);

#define Q_CHECK_PTR_RETURN(pointer, ...) \
do { \
    Q_CHECK_PTR(pointer); \
    if (pointer == nullptr) return __VA_ARGS__; \
} while (false);
