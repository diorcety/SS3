#ifndef INTERPOLATE2D_H_
#define INTERPOLATE2D_H_

#include <stdint.h>

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                 CONSTANTS                                                         *
 *                                                                                                                   *
 *********************************************************************************************************************/

#ifndef INTERPOLATE2D_MATH_TYPE
#define INTERPOLATE2D_MATH_TYPE float
#endif

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                  MACROS                                                           *
 *                                                                                                                   *
 *********************************************************************************************************************/

#define INTERPOLATE2D_MATH(x, x0, x1, y0, y1)                                                                          \
  ((y0) + ((INTERPOLATE2D_MATH_TYPE)((x) - (x0)) / (INTERPOLATE2D_MATH_TYPE)((x1) - (x0))) *                           \
              (INTERPOLATE2D_MATH_TYPE)((y1) - (y0)))

#define DEFINE_INTERPOLATE2D_TABLE(name, x_type, y_type)                                                               \
  typedef struct {                                                                                                     \
    const x_type *x;                                                                                                   \
    const y_type *y;                                                                                                   \
    unsigned int length;                                                                                               \
  } name

#define DECLARE_INTERPOLATE2D_FUNC(fname, table_type, x_type, y_type) y_type fname(const table_type *t, x_type x)

#define DEFINE_INTERPOLATE2D_FUNC(fname, table_type, x_type, y_type)                                                   \
  y_type fname(const table_type *t, x_type x) {                                                                        \
    if (!t || t->length < 2)                                                                                           \
      return (y_type)0;                                                                                                \
                                                                                                                       \
    unsigned int high = t->length - 1;                                                                                 \
                                                                                                                       \
    /* clamp */                                                                                                        \
    if (x <= t->x[0])                                                                                                  \
      return t->y[0];                                                                                                  \
    if (x >= t->x[high])                                                                                               \
      return t->y[high];                                                                                               \
                                                                                                                       \
    unsigned int low = 0;                                                                                              \
                                                                                                                       \
    /* binary search */                                                                                                \
    while ((high - low) > 1) {                                                                                         \
      unsigned int mid = (low + high) >> 1;                                                                            \
      if (x < t->x[mid])                                                                                               \
        high = mid;                                                                                                    \
      else                                                                                                             \
        low = mid;                                                                                                     \
    }                                                                                                                  \
                                                                                                                       \
    x_type x0 = t->x[low];                                                                                             \
    x_type x1 = t->x[high];                                                                                            \
    y_type y0 = t->y[low];                                                                                             \
    y_type y1 = t->y[high];                                                                                            \
                                                                                                                       \
    return (y_type)INTERPOLATE2D_MATH(x, x0, x1, y0, y1);                                                              \
  }

#endif // INTERPOLATE2D_H_
