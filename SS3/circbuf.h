/*
 * Copyright (c) 2020 Siddharth Chandrasekaran <siddharth@embedjournal.com>
 * Copyright (c) 2022 Yann Diorcet <diorcet.yann@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CIRCBUF_H_
#define CIRCBUF_H_

#include <stdbool.h>
#include <stdint.h>

/** --- Internal methods and structures. DON'T USE ---------------------------
 */
#define __CIRCBUF_DEPAREN(X) __CIRCBUF_ESC(__CIRCBUF_ISH X)
#define __CIRCBUF_ISH(...) __CIRCBUF_ISH __VA_ARGS__
#define __CIRCBUF_ESC(...) __CIRCBUF_ESC_(__VA_ARGS__)
#define __CIRCBUF_ESC_(...) __CIRCBUF_VAN##__VA_ARGS__
#define __CIRCBUF_VAN__CIRCBUF_ISH

#if defined(__cplusplus)
#define __CIRCBUF_TYPEDEF(TYPE, ...) using TYPE = __VA_ARGS__
#else
#define __CIRCBUF_TYPEDEF(TYPE, ...) typedef __VA_ARGS__ TYPE
#endif

#define __CIRCBUF_TYPE(name) __##name##_circbuf_t
#define __CIRCBUF_DEC_TYPE(name, type, size, stype)                                                                    \
  __CIRCBUF_TYPEDEF(__##name##_circ_size_t, __CIRCBUF_DEPAREN(stype));                                                 \
                                                                                                                       \
  typedef struct {                                                                                                     \
    __##name##_circ_size_t push_count;                                                                                 \
    __##name##_circ_size_t pop_count;                                                                                  \
    type buffer[size];                                                                                                 \
  } __CIRCBUF_TYPE(name);                                                                                              \
                                                                                                                       \
  bool __##name##_circbuf_pop(__CIRCBUF_TYPE(name) * circ_buf, type * elem, bool read_only);                           \
  bool __##name##_circbuf_push(__CIRCBUF_TYPE(name) * circ_buf, const type *elem);                                     \
  __##name##_circ_size_t __##name##_circbuf_used_space(__CIRCBUF_TYPE(name) * circ_buf);                               \
  __##name##_circ_size_t __##name##_circbuf_free_space(__CIRCBUF_TYPE(name) * circ_buf);

#define __CIRCBUF_SIZE(name, type, size, stype) (__##name##_circ_size_t)(size)
#define __CIRCBUF_LIMIT(name, type, size, stype) (__##name##_circ_size_t)(size * 2)

#define __CIRCBUF_ELEM(name, type, size, stype, count)                                                                 \
  circ_buf->buffer[(circ_buf->count >= __CIRCBUF_SIZE(name, type, size, stype)                                         \
                        ? circ_buf->count - __CIRCBUF_SIZE(name, type, size, stype)                                    \
                        : circ_buf->count)]

#define __CIRCBUF_TOTAL(name, type, size, stype, x)                                                                    \
  __##name##_circ_size_t x =                                                                                           \
      circ_buf->push_count >= circ_buf->pop_count                                                                      \
          ? (circ_buf->push_count - circ_buf->pop_count)                                                               \
          : ((__CIRCBUF_LIMIT(name, type, size, stype) - circ_buf->pop_count) + circ_buf->push_count);

#define __CIRCBUF_DEF_TYPE(name, type, size, stype)                                                                    \
                                                                                                                       \
  bool __##name##_circbuf_pop(__CIRCBUF_TYPE(name) * circ_buf, type * elem, bool read_only) {                          \
    __CIRCBUF_TOTAL(name, type, size, stype, total)                                                                    \
                                                                                                                       \
    if (total == 0)                                                                                                    \
      return false;                                                                                                    \
                                                                                                                       \
    if (elem)                                                                                                          \
      *elem = __CIRCBUF_ELEM(name, type, size, stype, pop_count);                                                      \
                                                                                                                       \
    if (!read_only) {                                                                                                  \
      circ_buf->pop_count++;                                                                                           \
      if (circ_buf->pop_count >= __CIRCBUF_LIMIT(name, type, size, stype))                                             \
        circ_buf->pop_count = 0;                                                                                       \
    }                                                                                                                  \
                                                                                                                       \
    return true;                                                                                                       \
  }                                                                                                                    \
                                                                                                                       \
  bool __##name##_circbuf_push(__CIRCBUF_TYPE(name) * circ_buf, const type *elem) {                                    \
    __CIRCBUF_TOTAL(name, type, size, stype, total)                                                                    \
                                                                                                                       \
    if (total >= __CIRCBUF_SIZE(name, type, size, stype))                                                              \
      return false;                                                                                                    \
                                                                                                                       \
    __CIRCBUF_ELEM(name, type, size, stype, push_count) = *elem;                                                       \
                                                                                                                       \
    circ_buf->push_count++;                                                                                            \
    if (circ_buf->push_count >= __CIRCBUF_LIMIT(name, type, size, stype))                                              \
      circ_buf->push_count = 0;                                                                                        \
                                                                                                                       \
    return true;                                                                                                       \
  }                                                                                                                    \
                                                                                                                       \
  __##name##_circ_size_t __##name##_circbuf_used_space(__CIRCBUF_TYPE(name) * circ_buf) {                              \
    __CIRCBUF_TOTAL(name, type, size, stype, total)                                                                    \
    return total;                                                                                                      \
  }                                                                                                                    \
                                                                                                                       \
  __##name##_circ_size_t __##name##_circbuf_free_space(__CIRCBUF_TYPE(name) * circ_buf) {                              \
    return __CIRCBUF_SIZE(name, type, size, stype) - __##name##_circbuf_used_space(circ_buf);                          \
  }

#define __CIRCBUF_DEC_NO_EXTERN(name, buf) __CIRCBUF_TYPE(name) buf;
#define __CIRCBUF_DEC(name, buf) extern __CIRCBUF_DEC_NO_EXTERN(name, buf)

#define __CIRCBUF_DEF(name, buf)                                                                                       \
  __CIRCBUF_TYPE(name)                                                                                                 \
  buf = {                                                                                                              \
      0,                                                                                                               \
      0,                                                                                                               \
      {},                                                                                                              \
  };

/* -------------------------------------------------------------------------- */

#if defined(__GNUC__) || defined(__clang__) || defined(__cplusplus)

#if defined(__cplusplus)
#include <type_traits>
#define __CIRCBUF_TYPEOF(_literal)                                                                                     \
  (std::conditional_t<(_literal) <= UINT8_MAX,                                                                         \
                      uint8_t,                                                                                         \
                      std::conditional_t<(_literal) <= UINT16_MAX,                                                     \
                                         uint16_t,                                                                     \
                                         std::conditional_t<(_literal) <= UINT32_MAX, uint32_t, uint64_t>>>)
#else
#define __CIRCBUF_TYPEOF(_literal)                                                                                     \
  typeof(__builtin_choose_expr(                                                                                        \
      (_literal) <= UINT8_MAX,                                                                                         \
      (uint8_t)0,                                                                                                      \
      __builtin_choose_expr((_literal) <= UINT16_MAX,                                                                  \
                            (uint16_t)0,                                                                               \
                            __builtin_choose_expr((_literal) <= UINT32_MAX, (uint32_t)0, (uint64_t)0))))
#endif

/**
 * Description:
 *   Declares a global circular buffer type `name` of a given type and size.
 *   The type can be native data types or user-defined data types.
 *
 * Usage:
 *   CIRCBUF_DEC(UINT8_13, uint8_t, 13);
 *   CIRCBUF_DEC(STRUCT_FOO_10, struct foo, 10);
 */
#define CIRCBUF_DEC_TYPE(name, type, size) __CIRCBUF_DEC_TYPE(name, type, size, __CIRCBUF_TYPEOF(size * 2))

/**
 * Description:
 *   Defines a global circular buffer type `name` of a given type and size.
 *   The type can be native data types or user-defined data types.
 *
 * Usage:
 *   CIRCBUF_DEF(UINT8_13, uint8_t, byte_buf, 13);
 *   CIRCBUF_DEF(STRUCT_FOO_10, struct foo, 10);
 */
#define CIRCBUF_DEF_TYPE(name, type, size) __CIRCBUF_DEF_TYPE(name, type, size, __CIRCBUF_TYPEOF(size * 2))

/**
 * Description:
 *   Declares and defines a global circular buffer type `name` of a given type
 *   and size. The type can be native data types or user-defined data types.
 *
 * Usage:
 *   CIRCBUF_DEC_N_DEF_TYPE(UINT8_13, uint8_t, 13);
 *   CIRCBUF_DEC_N_DEF_TYPE(STRUCT_FOO_10, struct foo, 10);
 */
#define CIRCBUF_DEC_N_DEF_TYPE(name, type, size)                                                                       \
  CIRCBUF_DEC_TYPE(name, type, size)                                                                                   \
  CIRCBUF_DEF_TYPE(name, type, size)

#else

/**
 * Description:
 *   Declares a global circular buffer type `name` of a given type and size with
 *   `stype` an unsigned integer type which can contain twice the value of the
 *   size. The type can be native data types or user-defined data types.
 *
 * Usage:
 *   CIRCBUF_DEC_TYPE(UINT8_13, uint8_t, 13, uint8_t);
 *   CIRCBUF_DEC_TYPE(STRUCT_FOO_10, struct foo, 10, uint8_t);
 */
#define CIRCBUF_DEC_TYPE(name, type, size, stype) __CIRCBUF_DEC_TYPE(name, type, size, stype)

/**
 * Description:
 *   Defines a global circular buffer type `name` of a given type and size with
 *   `stype` an unsigned integer type which can contain twice the value of the
 *   size. The type can be native data types or user-defined data types.
 *
 * Usage:
 *   CIRCBUF_DEF_TYPE(UINT8_13, uint8_t, byte_buf, 13, uint8_t);
 *   CIRCBUF_DEF_TYPE(STRUCT_FOO_10, struct foo, 10, uint8_t);
 */
#define CIRCBUF_DEF_TYPE(name, type, size, stype) __CIRCBUF_DEF_TYPE(name, type, size, stype)

/**
 * Description:
 *   Declares and defines a global circular buffer type `name` of a given type
 *   and size with `stype` an unsigned integer type which can contain twice the
 *   value of the size. The type can be native data types or user-defined data
 *   types.
 *
 * Usage:
 *   CIRCBUF_DEC_N_DEF_TYPE(UINT8_13, uint8_t, 13, uint8_t);
 *   CIRCBUF_DEC_N_DEF_TYPE(STRUCT_FOO_10, struct foo, 10, uint8_t);
 */
#define CIRCBUF_DEC_N_DEF_TYPE(name, type, size, stype)                                                                \
  CIRCBUF_DEC_TYPE(name, type, size, stype)                                                                            \
  CIRCBUF_DEF_TYPE(name, type, size, stype)
#endif

/**
 * Description:
 *   Declares a global circular buffer `buf` of a `name` type.
 *
 * Usage:
 *   CIRCBUF_DEC(UINT8_13, byte_buf);
 *   CIRCBUF_DEC(STRUCT_FOO_10, foo_buf);
 */
#define CIRCBUF_DEC(name, buf) __CIRCBUF_DEC(name, buf)

/**
 * Description:
 *   Defines a global circular buffer `buf` of a `name` type.
 *
 * Usage:
 *   CIRCBUF_DEF(UINT8_13, byte_buf);
 *   CIRCBUF_DEF(STRUCT_FOO_10, foo_buf);
 */
#define CIRCBUF_DEF(name, buf) __CIRCBUF_DEF(name, buf)

/**
 * Description:
 *   Declares and defines a global circular buffer `buf` of a `name` type.
 *
 * Usage:
 *   CIRCBUF_DEC_N_DEF(UINT8_13, byte_buf);
 *   CIRCBUF_DEC_N_DEF(STRUCT_FOO_10, foo_buf);
 */
#define CIRCBUF_DEC_N_DEF(name, buf)                                                                                   \
  CIRCBUF_DEC(name, buf)                                                                                               \
  CIRCBUF_DEF(name, buf)

/**
 * Description:
 *   Resets the circular buffer offsets to zero.
 */
#define CIRCBUF_FLUSH(name, buf)                                                                                       \
  do {                                                                                                                 \
    buf.push_count = 0;                                                                                                \
    buf.pop_count = 0;                                                                                                 \
  } while (0)

/**
 * Description:
 *   Pushes element pointed to by `elem` at the head of circular buffer `buf`.
 *   This is read-write method, occupancy count increases by one.
 *
 * Returns (int):
 *   true - Success
 *   false - Out of space
 */
#define CIRCBUF_PUSH(name, buf, elem) __##name##_circbuf_push(&buf, &elem)

/**
 * Description:
 *   Removes the element at tail from circular buffer `buf` and makes it
 *   available at `elem`. This is read-write method, occupancy count reduces
 *   by one.
 *
 * Returns (int):
 *   true - Success
 *   false - Empty
 */
#define CIRCBUF_PEEK(name, buf, elem) __##name##_circbuf_pop(&buf, &elem, 1)

/**
 * Description:
 *   Copies the element at tail of circular buffer `buf` into location pointed
 *   by `elem`. This method is read-only, does not later occupancy status.
 *
 * Returns (int):
 *   true - Success
 *   false - Empty
 */
#define CIRCBUF_POP(name, buf, elem) __##name##_circbuf_pop(&buf, &elem, 0)

/**
 * Description:
 *   Returns the number of free slots in the circular buffer `buf`.
 *
 * Returns (int):
 *   0..N - number of slots available.
 */
#define CIRCBUF_FS(name, buf) __##name##_circbuf_free_space(&buf)

/**
 * Description:
 *   Returns the number of used slots in the circular buffer `buf`.
 *
 * Returns (int):
 *   0..N - number of slots used.
 */
#define CIRCBUF_US(name, buf) __##name##_circbuf_used_space(&buf)

#endif /* CIRCBUF_H_ */