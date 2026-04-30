#ifndef UTIL_H_
#define UTIL_H_

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                  MACROS                                                           *
 *                                                                                                                   *
 *********************************************************************************************************************/

//
// Maths
//
#define ABS(x) ((x) < 0 ? -(x) : (x))
#define MIN(a, b) _Generic((a), int: min_int)(a, b)
#define MAX(a, b) _Generic((a), int: max_int)(a, b)
#define ROUND(t, x) ((t)((x) + 0.5f))

#define IIR_FILTER_ADD(window, acc, value)                                                                             \
  ((acc) = ((acc) + ((value) * (window)) - (((acc) + ((window) / 2)) / (window))))
#define IIR_FILTER_GET(window, acc) (((acc) + (((window) * (window)) / 2)) / ((window) * (window)))

//
// Arrays
//
#define ARRAY_SIZE(ARR) (sizeof(ARR) / (sizeof(ARR[0])))

//
// Electronic circuits
//
#define VOLTAGE_DIVIDER(UPPER, LOWER) ((LOWER) / ((LOWER) + (UPPER)))

// Miscs
#define UNUSED(x) ((void)(x))
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

/*********************************************************************************************************************
 *                                                                                                                   *
 *                                                 FUNCTIONS                                                         *
 *                                                                                                                   *
 *********************************************************************************************************************/

static inline int min_int(int a, int b) { return (a > b) ? b : a; }

static inline int max_int(int a, int b) { return (a > b) ? a : b; }

#endif // UTIL_H_
