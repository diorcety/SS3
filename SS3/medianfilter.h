/*
 * Copyright (c) 2011 ashelly.myopenid.com
 * Copyright (c) 2022 Yann Diorcet <diorcet.yann@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef MEDIANFILTER_H_
#define MEDIANFILTER_H_

#include <stdbool.h>
#include <stdint.h>

/** --- Internal methods and structures. DON'T USE ---------------------------
 */

#define __MEDIAN_FILTER_DEPAREN(X) __MEDIAN_FILTER_ESC(__MEDIAN_FILTER_ISH X)
#define __MEDIAN_FILTER_ISH(...) __MEDIAN_FILTER_ISH __VA_ARGS__
#define __MEDIAN_FILTER_ESC(...) __MEDIAN_FILTER_ESC_(__VA_ARGS__)
#define __MEDIAN_FILTER_ESC_(...) __MEDIAN_FILTER_VAN##__VA_ARGS__
#define __MEDIAN_FILTER_VAN__MEDIAN_FILTER_ISH

#if defined(__cplusplus)
#define __MEDIAN_FILTER_TYPEDEF(TYPE, ...) using TYPE = __VA_ARGS__
#else
#define __MEDIAN_FILTER_TYPEDEF(TYPE, ...) typedef __VA_ARGS__ TYPE
#endif

#define __MEDIAN_FILTER_ITEMLESS(name, a, b) ((a) < (b))
#define __MEDIAN_FILTER_ITEMMEAN(name, a, b) ((__##name##_type_t)(((a) + (b)) / 2))
#define __MEDIAN_FILTER_MINCT(name, m) ((__##name##_off_t)(((m)->ct - 1) / 2))
#define __MEDIAN_FILTER_MAXCT(name, m) ((__##name##_off_t)(((m)->ct) / 2))

#define __MEDIAN_FILTER_DEC_TYPE(name, type, size, stype)                                                              \
  __MEDIAN_FILTER_TYPEDEF(__##name##_off_t, __MEDIAN_FILTER_DEPAREN(stype));                                           \
  __MEDIAN_FILTER_TYPEDEF(__##name##_type_t, __MEDIAN_FILTER_DEPAREN(type));                                           \
  typedef struct __##name##_t {                                                                                        \
    __##name##_off_t idx;                                                                                              \
    __##name##_off_t ct;                                                                                               \
    __##name##_off_t *pos;                                                                                             \
    __##name##_off_t *heap;                                                                                            \
    __##name##_type_t data[(size)];                                                                                    \
    __##name##_off_t storage[(size) * 2];                                                                              \
  } __##name##_t;                                                                                                      \
                                                                                                                       \
  void __##name##_init(__##name##_t *m);                                                                               \
  void __##name##_insert(__##name##_t *m, __##name##_type_t v);                                                        \
  bool __##name##_median(__##name##_t *m, __##name##_type_t *v, bool mean);                                            \
  __##name##_off_t __##name##_mmless(__##name##_t *m, __##name##_off_t i, __##name##_off_t j);                         \
  __##name##_off_t __##name##_mmexchange(__##name##_t *m, __##name##_off_t i, __##name##_off_t j);                     \
  __##name##_off_t __##name##_mmcmpexch(__##name##_t *m, __##name##_off_t i, __##name##_off_t j);                      \
  void __##name##_minsortdown(__##name##_t *m, __##name##_off_t i);                                                    \
  void __##name##_maxsortdown(__##name##_t *m, __##name##_off_t i);                                                    \
  __##name##_off_t __##name##_minsortup(__##name##_t *m, __##name##_off_t i);                                          \
  __##name##_off_t __##name##_maxsortup(__##name##_t *m, __##name##_off_t i);

#define __MEDIAN_FILTER_DEF_TYPE(name, type, size, stype)                                                              \
  void __##name##_init(__##name##_t *m) {                                                                              \
    __##name##_off_t i = (size);                                                                                       \
    m->ct = m->idx = 0;                                                                                                \
    m->pos = m->storage;                                                                                               \
    m->heap = m->pos + (size) + ((size) / 2);                                                                          \
    while (i--) {                                                                                                      \
      m->pos[i] = ((i + 1) / 2) * ((i & 1) ? -1 : 1);                                                                  \
      m->heap[m->pos[i]] = i;                                                                                          \
    }                                                                                                                  \
  }                                                                                                                    \
  void __##name##_insert(__##name##_t *m, __##name##_type_t v) {                                                       \
    bool is_new = (m->ct < (size));                                                                                    \
    __##name##_off_t p = m->pos[m->idx];                                                                               \
    __##name##_type_t old = m->data[m->idx];                                                                           \
    m->data[m->idx] = v;                                                                                               \
    m->idx = (m->idx + 1) % (size);                                                                                    \
    m->ct += is_new;                                                                                                   \
    if (p > 0) {                                                                                                       \
      if (!is_new && __MEDIAN_FILTER_ITEMLESS(name, old, v)) {                                                         \
        __##name##_minsortdown(m, p * 2);                                                                              \
      } else if (__##name##_minsortup(m, p)) {                                                                         \
        __##name##_maxsortdown(m, -1);                                                                                 \
      }                                                                                                                \
    } else if (p < 0) {                                                                                                \
      if (!is_new && __MEDIAN_FILTER_ITEMLESS(name, v, old)) {                                                         \
        __##name##_maxsortdown(m, p * 2);                                                                              \
      } else if (__##name##_maxsortup(m, p)) {                                                                         \
        __##name##_minsortdown(m, 1);                                                                                  \
      }                                                                                                                \
    } else {                                                                                                           \
      if (__MEDIAN_FILTER_MAXCT(name, m)) {                                                                            \
        __##name##_maxsortdown(m, -1);                                                                                 \
      }                                                                                                                \
      if (__MEDIAN_FILTER_MINCT(name, m)) {                                                                            \
        __##name##_minsortdown(m, 1);                                                                                  \
      }                                                                                                                \
    }                                                                                                                  \
  }                                                                                                                    \
  bool __##name##_median(__##name##_t *m, __##name##_type_t *v, bool mean) {                                           \
    if (m->ct == 0)                                                                                                    \
      return false;                                                                                                    \
    *v = m->data[m->heap[0]];                                                                                          \
    if (mean && (m->ct & 1) == 0) {                                                                                    \
      *v = __MEDIAN_FILTER_ITEMMEAN(name, *v, m->data[m->heap[-1]]);                                                   \
    }                                                                                                                  \
    return true;                                                                                                       \
  }                                                                                                                    \
  __##name##_off_t __##name##_mmless(__##name##_t *m, __##name##_off_t i, __##name##_off_t j) {                        \
    return __MEDIAN_FILTER_ITEMLESS(name, m->data[m->heap[i]], m->data[m->heap[j]]);                                   \
  }                                                                                                                    \
  __##name##_off_t __##name##_mmexchange(__##name##_t *m, __##name##_off_t i, __##name##_off_t j) {                    \
    __##name##_off_t t = m->heap[i];                                                                                   \
    m->heap[i] = m->heap[j];                                                                                           \
    m->heap[j] = t;                                                                                                    \
    m->pos[m->heap[i]] = i;                                                                                            \
    m->pos[m->heap[j]] = j;                                                                                            \
    return 1;                                                                                                          \
  }                                                                                                                    \
  __##name##_off_t __##name##_mmcmpexch(__##name##_t *m, __##name##_off_t i, __##name##_off_t j) {                     \
    return (__##name##_mmless(m, i, j) && __##name##_mmexchange(m, i, j));                                             \
  }                                                                                                                    \
  void __##name##_minsortdown(__##name##_t *m, __##name##_off_t i) {                                                   \
    for (; i <= __MEDIAN_FILTER_MINCT(name, m); i *= 2) {                                                              \
      if (i > 1 && i < __MEDIAN_FILTER_MINCT(name, m) && __##name##_mmless(m, i + 1, i)) {                             \
        ++i;                                                                                                           \
      }                                                                                                                \
      if (!__##name##_mmcmpexch(m, i, i / 2)) {                                                                        \
        break;                                                                                                         \
      }                                                                                                                \
    }                                                                                                                  \
  }                                                                                                                    \
  void __##name##_maxsortdown(__##name##_t *m, __##name##_off_t i) {                                                   \
    for (; i >= -__MEDIAN_FILTER_MAXCT(name, m); i *= 2) {                                                             \
      if (i < -1 && i > -__MEDIAN_FILTER_MAXCT(name, m) && __##name##_mmless(m, i, i - 1)) {                           \
        --i;                                                                                                           \
      }                                                                                                                \
      if (!__##name##_mmcmpexch(m, i / 2, i)) {                                                                        \
        break;                                                                                                         \
      }                                                                                                                \
    }                                                                                                                  \
  }                                                                                                                    \
  __##name##_off_t __##name##_minsortup(__##name##_t *m, __##name##_off_t i) {                                         \
    while (i > 0 && __##name##_mmcmpexch(m, i, i / 2))                                                                 \
      i /= 2;                                                                                                          \
    return (i == 0);                                                                                                   \
  }                                                                                                                    \
  __##name##_off_t __##name##_maxsortup(__##name##_t *m, __##name##_off_t i) {                                         \
    while (i < 0 && __##name##_mmcmpexch(m, i / 2, i))                                                                 \
      i /= 2;                                                                                                          \
    return (i == 0);                                                                                                   \
  }

#define __MEDIAN_FILTER_DEC(name, var) extern __##name##_t var
#define __MEDIAN_FILTER_DEF(name, var) __##name##_t var

/* -------------------------------------------------------------------------- */

#if defined(__GNUC__) || defined(__cplusplus)

#if defined(__cplusplus)
#include <type_traits>
#define __MEDIAN_FILTER_TYPEOF(_literal)                                                                               \
  (std::conditional_t<_literal <= INT8_MAX,                                                                            \
                      int8_t,                                                                                          \
                      std::conditional_t<_literal <= INT16_MAX,                                                        \
                                         int16_t,                                                                      \
                                         std::conditional_t<_literal <= INT32_MAX, int32_t, int64_t>>>)
#else
#define __MEDIAN_FILTER_TYPEOF(_literal)                                                                               \
  typeof(__builtin_choose_expr(                                                                                        \
      (_literal) <= INT8_MAX,                                                                                          \
      (int8_t)0,                                                                                                       \
      __builtin_choose_expr((_literal) <= INT16_MAX,                                                                   \
                            (int16_t)0,                                                                                \
                            __builtin_choose_expr((_literal) <= INT32_MAX, (int32_t)0, (int64_t)0))))
#endif

/**
 * Description:
 *   Declares and defines a median filter `name` of a given type and size.
 *   The type can be any type supporting comparisons and arithmetic
 *   operations.
 *
 * Usage:
 *   MEDIAN_FILTER_DEC_TYPE(UINT8_13, uint8_t, 13);
 *   MEDIAN_FILTER_DEC_TYPE(FLOAT_5, float, 5);
 */
#define MEDIAN_FILTER_DEC_TYPE(name, type, size)                                                                       \
  __MEDIAN_FILTER_DEC_TYPE(name, type, size, __MEDIAN_FILTER_TYPEOF(size))

/**
 * Description:
 *   Declares and defines a median filter `name` of a given type and size.
 *   The type can be any type supporting comparisons and arithmetic
 *   operations.
 *
 * Usage:
 *   MEDIAN_FILTER_DEF_TYPE(UINT8_13, uint8_t, 13);
 *   MEDIAN_FILTER_DEF_TYPE(FLOAT_5, float, 5);
 */
#define MEDIAN_FILTER_DEF_TYPE(name, type, size)                                                                       \
  __MEDIAN_FILTER_DEF_TYPE(name, type, size, __MEDIAN_FILTER_TYPEOF(size))

/**
 * Description:
 *   Declares and defines a median filter `name` of a given type and size.
 *   The type can be any type supporting comparisons and arithmetic
 *   operations.
 *
 * Usage:
 *   MEDIAN_FILTER_DEC_NDEF_TYPE(UINT8_13, uint8_t, 13);
 *   MEDIAN_FILTER_DEC_NDEF_TYPE(FLOAT_5, float, 5);
 */
#define MEDIAN_FILTER_DEC_NDEF_TYPE(name, type, size)                                                                  \
  MEDIAN_FILTER_DEC_TYPE(name, type, size)                                                                             \
  MEDIAN_FILTER_DEF_TYPE(name, type, size)
#else

/**
 * Description:
 *   Declares a median filter `name` of a given type and size with `stype` an
 *   signed integer type which can contain twice the value of the size. The
 *   type can be any type supporting comparisons and arithmetic
 *   operations.
 *
 * Usage:
 *   MEDIAN_FILTER_DEC_TYPE(UINT8_13, uint8_t, 13, int8_t);
 *   MEDIAN_FILTER_DEC_TYPE(FLOAT_5, float, 5, int8_t);
 */
#define MEDIAN_FILTER_DEC_TYPE(name, type, size, stype) __MEDIAN_FILTER_DEC_TYPE(name, type, size, stype)

/**
 * Description:
 *   Defines a median filter `name` of a given type and size with `stype` an
 *   signed integer type which can contain twice the value of the size. The
 *   type can be any type supporting comparisons and arithmetic
 *   operations.
 *
 * Usage:
 *   MEDIAN_FILTER_DEF_TYPE(UINT8_13, uint8_t, 13, int8_t);
 *   MEDIAN_FILTER_DEF_TYPE(FLOAT_5, float, 5, int8_t);
 */
#define MEDIAN_FILTER_DEF_TYPE(name, type, size, stype) __MEDIAN_FILTER_DEF_TYPE(name, type, size, stype)

/**
 * Description:
 *   Declares and defines a median filter `name` of a given type and size with
 *   `stype` an signed integer type which can contain twice the value of the
 *   size. The type can be any type supporting comparisons and arithmetic
 *   operations.
 *
 * Usage:
 *   MEDIAN_FILTER_DEC_NDEF_TYPE(UINT8_13, uint8_t, 13, int8_t);
 *   MEDIAN_FILTER_DEC_NDEF_TYPE(FLOAT_5, float, 5, int8_t);
 */
#define MEDIAN_FILTER_DEC_NDEF_TYPE(name, type, size, stype)                                                           \
  MEDIAN_FILTER_DEC_TYPE(name, type, size, stype)                                                                      \
  MEDIAN_FILTER_DEF_TYPE(name, type, size, stype)
#endif

/**
 * Description:
 *   Declares a median filter `var` of a `name` type.
 *
 * Usage:
 *   MEDIAN_FILTER_DEC(UINT8_13, mf_1);
 *   MEDIAN_FILTER_DEC(FLOAT_5, mf_2);
 */
#define MEDIAN_FILTER_DEC(name, var) __MEDIAN_FILTER_DEC(name, var)

/**
 * Description:
 *   Defines a median filter `var` of a `name` type.
 *
 * Usage:
 *   MEDIAN_FILTER_DEF(UINT8_13, mf_1);
 *   MEDIAN_FILTER_DEF(FLOAT_5, mf_2);
 */
#define MEDIAN_FILTER_DEF(name, var) __MEDIAN_FILTER_DEF(name, var)

/**
 * Description:
 *   Declares and defines a median filter `var` of a `name` type.
 *
 * Usage:
 *   MEDIAN_FILTER_DEC_N_DEF(UINT8_13, mf_1);
 *   MEDIAN_FILTER_DEC_N_DEF(FLOAT_5, mf_2);
 */
#define MEDIAN_FILTER_DEC_N_DEF(name, var)                                                                             \
  MEDIAN_FILTER_DEC(name, var)                                                                                         \
  MEDIAN_FILTER_DEF(name, var)

/**
 * Description:
 *   Initialize `var` median filter.
 */
#define _MEDIAN_FILTER_INIT(name, var) __##name##_init(&var)
#define MEDIAN_FILTER_INIT(name, var) _MEDIAN_FILTER_INIT(name, var)

/**
 * Description:
 *   Insert `value` inside `var` median filter.
 */
#define _MEDIAN_FILTER_INSERT(name, var, v) __##name##_insert(&var, v)
#define MEDIAN_FILTER_INSERT(name, var, v) _MEDIAN_FILTER_INSERT(name, var, v)

/**
 * Description:
 *   Get median value from `var` and makes it available at `value`.
 *   If mean is true and the number of values in the median filter is even,
 *   a mean value is computed between two median values.
 *
 * Returns (int):
 *   true - Success
 *   false - No data
 */
#define _MEDIAN_FILTER_MEDIAN(name, var, value, mean) __##name##_median(&var, &value, mean)
#define MEDIAN_FILTER_MEDIAN(name, var, value, mean) _MEDIAN_FILTER_MEDIAN(name, var, value, mean)

#endif
