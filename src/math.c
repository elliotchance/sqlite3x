// This file is not part of the core sqlite3 codebase. It is part of the
// github.com/elliotchance/sqlite3x project.
//
// This file provides more math functions than are otherwise not included with
// sqlite3.

#include "sqliteInt.h"
#include <math.h>

// cos(X) returns the cosine of *X* (*X* is expressed in radians). If *X* is
// `NULL`, then the result will be `NULL`.
//
// If *X* not numeric then it will be treated internally as `0.0`. This will
// produce a result of `1.0`.
static void cosFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  assert(argc == 1);
  UNUSED_PARAMETER(argc);

  // If the argument is NULL, the result must also be NULL.
  if (sqlite3_value_type(argv[0]) == SQLITE_NULL) {
    sqlite3_result_null(context);
    return;
  }

  double x = sqlite3_value_double(argv[0]);
  sqlite3_result_double(context, cos(x));
}

// sin(X) returns the sine of *X* (*X* is expressed in radians). If *X* is
// `NULL`, then the result will be `NULL`.
//
// If *X* not numeric then it will be treated internally as `0.0`. This will
// produce a result of `0.0`.
static void sinFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  assert(argc == 1);
  UNUSED_PARAMETER(argc);

  // If the argument is NULL, the result must also be NULL.
  if (sqlite3_value_type(argv[0]) == SQLITE_NULL) {
    sqlite3_result_null(context);
    return;
  }

  double x = sqlite3_value_double(argv[0]);
  sqlite3_result_double(context, sin(x));
}

// tan(X) returns the tangent of *X* (*X* is expressed in radians). If *X* is
// `NULL`, then the result will be `NULL`.
//
// If *X* not numeric then it will be treated internally as `0.0`. This will
// produce a result of `0.0`.
static void tanFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  assert(argc == 1);
  UNUSED_PARAMETER(argc);

  // If the argument is NULL, the result must also be NULL.
  if (sqlite3_value_type(argv[0]) == SQLITE_NULL) {
    sqlite3_result_null(context);
    return;
  }

  double x = sqlite3_value_double(argv[0]);
  sqlite3_result_double(context, tan(x));
}

// pow(X,Y) returns *X* to the power of *Y*. The result is always a
// floating-point value. `NULL` is returned if either of the inputs are `NULL`.
//
// You should be careful that inputs are valid and sensible. pow(X,Y) does not
// provide any protection for values that are invalid (not valid numbers).
// Result values that are too small or large (overflow) will have unexpected
// behavior depending on your system.
static void powFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  assert(argc == 2);
  UNUSED_PARAMETER(argc);

  // If either argument is NULL, the result also must be NULL.
  if (sqlite3_value_type(argv[0]) == SQLITE_NULL ||
      sqlite3_value_type(argv[1]) == SQLITE_NULL) {
    sqlite3_result_null(context);
    return;
  }

  // All operations are handled as floating-point. If either of the inputs
  // cannot be resolved to a float then the result is unexpected.
  //
  // It might be nice to have pow() return an integer if both inputs are
  // integers. I did try this initially, but ran into some problems detecting
  // integer overflows when the result is just ove the max signed int64 (see
  // LARGEST_INT64). Without being able to correctly detect the overflow it
  // would produce bad results, so it's paramount if its ever implemented again
  // that there is a test case for "pow(2, 63)".
  double x = sqlite3_value_double(argv[0]);
  double y = sqlite3_value_double(argv[1]);
  sqlite3_result_double(context, pow(x, y));
}

// pi() returns the value of pi, which is approximately 3.14159.
static void piFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  assert(argc == 0);
  UNUSED_PARAMETER(argc);

  sqlite3_result_double(context, M_PI);
}

// e() returns the value of e, which is approximately 2.71828.
static void eFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  assert(argc == 0);
  UNUSED_PARAMETER(argc);

  sqlite3_result_double(context, M_E);
}

// acos(X) returns the arc-cosine of *X*. If *X* is `NULL`, then the result will
// be `NULL`.
//
// If *X* not numeric then it will be treated internally as `0.0`. This will
// produce a result of `1.5707963267949`.
static void acosFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  assert(argc == 1);
  UNUSED_PARAMETER(argc);

  // If the argument is NULL, the result must also be NULL.
  if (sqlite3_value_type(argv[0]) == SQLITE_NULL) {
    sqlite3_result_null(context);
    return;
  }

  double x = sqlite3_value_double(argv[0]);
  sqlite3_result_double(context, acos(x));
}

// asin(X) returns the arc-sine of *X*. If *X* is `NULL`, then the result will
// be `NULL`.
//
// If *X* not numeric then it will be treated internally as `0.0`. This will
// produce a result of `0.0`.
static void asinFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  assert(argc == 1);
  UNUSED_PARAMETER(argc);

  // If the argument is NULL, the result must also be NULL.
  if (sqlite3_value_type(argv[0]) == SQLITE_NULL) {
    sqlite3_result_null(context);
    return;
  }

  double x = sqlite3_value_double(argv[0]);
  sqlite3_result_double(context, asin(x));
}

// atan(X) returns the arc-tangent of *X*. If *X* is `NULL`, then the result
// will be `NULL`.
//
// If *X* not numeric then it will be treated internally as `0.0`. This will
// produce a result of `0.0`.
static void atanFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  assert(argc == 1);
  UNUSED_PARAMETER(argc);

  // If the argument is NULL, the result must also be NULL.
  if (sqlite3_value_type(argv[0]) == SQLITE_NULL) {
    sqlite3_result_null(context);
    return;
  }

  double x = sqlite3_value_double(argv[0]);
  sqlite3_result_double(context, atan(x));
}

// atan2(Y,X) returns arc-tangent with *Y* and *X*. `NULL` is returned if either
// of the inputs are `NULL`.
//
// You should be careful that inputs are valid and sensible. atan2(Y,X) does not
// provide any protection for values that are invalid (not valid numbers).
static void atan2Func(sqlite3_context *context, int argc, sqlite3_value **argv) {
  assert(argc == 2);
  UNUSED_PARAMETER(argc);

  // If either argument is NULL, the result also must be NULL.
  if (sqlite3_value_type(argv[0]) == SQLITE_NULL ||
      sqlite3_value_type(argv[1]) == SQLITE_NULL) {
    sqlite3_result_null(context);
    return;
  }

  double y = sqlite3_value_double(argv[0]);
  double x = sqlite3_value_double(argv[1]);
  sqlite3_result_double(context, atan2(y, x));
}

// cosh(X) returns the hyperbolic cosine of *X*. If *X* is `NULL`, then the
// result will be `NULL`.
//
// If *X* not numeric then it will be treated internally as `0.0`. This will
// produce a result of `0.0`.
static void coshFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  assert(argc == 1);
  UNUSED_PARAMETER(argc);

  // If the argument is NULL, the result must also be NULL.
  if (sqlite3_value_type(argv[0]) == SQLITE_NULL) {
    sqlite3_result_null(context);
    return;
  }

  double x = sqlite3_value_double(argv[0]);
  sqlite3_result_double(context, cosh(x));
}

// sinh(X) returns the hyperbolic sine of *X*. If *X* is `NULL`, then the result
// will be `NULL`.
//
// If *X* not numeric then it will be treated internally as `0.0`. This will
// produce a result of `0.0`.
static void sinhFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  assert(argc == 1);
  UNUSED_PARAMETER(argc);

  // If the argument is NULL, the result must also be NULL.
  if (sqlite3_value_type(argv[0]) == SQLITE_NULL) {
    sqlite3_result_null(context);
    return;
  }

  double x = sqlite3_value_double(argv[0]);
  sqlite3_result_double(context, sinh(x));
}

// tanh(X) returns the hyperbolic tangent of *X*. If *X* is `NULL`, then the
// result will be `NULL`.
//
// If *X* not numeric then it will be treated internally as `0.0`. This will
// produce a result of `0.0`.
static void tanhFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  assert(argc == 1);
  UNUSED_PARAMETER(argc);

  // If the argument is NULL, the result must also be NULL.
  if (sqlite3_value_type(argv[0]) == SQLITE_NULL) {
    sqlite3_result_null(context);
    return;
  }

  double x = sqlite3_value_double(argv[0]);
  sqlite3_result_double(context, tanh(x));
}

// exp(X) returns *e* to the power of *X*. If *X* is `NULL`, then the result
// will be `NULL`.
//
// If *X* not numeric then it will be treated internally as `0.0`. This will
// produce a result of `1.0`.
//
// exp(X) does not provide any protection for result values that are too small
// or large (overflow) will have unexpected behavior depending on your system.
static void expFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  assert(argc == 1);
  UNUSED_PARAMETER(argc);

  // If the argument is NULL, the result must also be NULL.
  if (sqlite3_value_type(argv[0]) == SQLITE_NULL) {
    sqlite3_result_null(context);
    return;
  }

  double x = sqlite3_value_double(argv[0]);
  sqlite3_result_double(context, exp(x));
}

// log(X) returns natural logarithm (base *e*) of *X*. If *X* is `NULL`, then
// the result will be `NULL`.
//
// If *X* not numeric then it will be treated internally as `0.0`. This will
// produce a result of `-Inf`.
static void logFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  assert(argc == 1);
  UNUSED_PARAMETER(argc);

  // If the argument is NULL, the result must also be NULL.
  if (sqlite3_value_type(argv[0]) == SQLITE_NULL) {
    sqlite3_result_null(context);
    return;
  }

  double x = sqlite3_value_double(argv[0]);
  sqlite3_result_double(context, log(x));
}

// log10(X) returns common logarithm (base 10) of *X*. If *X* is `NULL`, then
// the result will be `NULL`.
//
// If *X* not numeric then it will be treated internally as `0.0`. This will
// produce a result of `-Inf`.
static void log10Func(sqlite3_context *context, int argc, sqlite3_value **argv) {
  assert(argc == 1);
  UNUSED_PARAMETER(argc);

  // If the argument is NULL, the result must also be NULL.
  if (sqlite3_value_type(argv[0]) == SQLITE_NULL) {
    sqlite3_result_null(context);
    return;
  }

  double x = sqlite3_value_double(argv[0]);
  sqlite3_result_double(context, log10(x));
}

// sqrt(X) returns the square root of *X*. If *X* is `NULL`, then the result
// will be `NULL`.
//
// If *X* not numeric then it will be treated internally as `0.0`. This will
// produce a result of `0.0`.
static void sqrtFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  assert(argc == 1);
  UNUSED_PARAMETER(argc);

  // If the argument is NULL, the result must also be NULL.
  if (sqlite3_value_type(argv[0]) == SQLITE_NULL) {
    sqlite3_result_null(context);
    return;
  }

  double x = sqlite3_value_double(argv[0]);
  sqlite3_result_double(context, sqrt(x));
}

// ceil(X) rounds *X* upward, returning the smallest integral value that is not
// less than *X*. If *X* is `NULL`, then the result will be `NULL`, otherwise
// the result will always be a floating-point value.
//
// If *X* not numeric then it will be treated internally as `0.0`. This will
// produce a result of `0.0`.
static void ceilFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  assert(argc == 1);
  UNUSED_PARAMETER(argc);

  // If the argument is NULL, the result must also be NULL.
  if (sqlite3_value_type(argv[0]) == SQLITE_NULL) {
    sqlite3_result_null(context);
    return;
  }

  double x = sqlite3_value_double(argv[0]);
  sqlite3_result_double(context, ceil(x));
}

// floor(X) rounds *X* downward, returning the largest integral value that is
// not greater than *X*. If *X* is `NULL`, then the result will be `NULL`,
// otherwise the result will always be a floating-point value.
//
// If *X* not numeric then it will be treated internally as `0.0`. This will
// produce a result of `0.0`.
static void floorFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  assert(argc == 1);
  UNUSED_PARAMETER(argc);

  // If the argument is NULL, the result must also be NULL.
  if (sqlite3_value_type(argv[0]) == SQLITE_NULL) {
    sqlite3_result_null(context);
    return;
  }

  double x = sqlite3_value_double(argv[0]);
  sqlite3_result_double(context, floor(x));
}

// Register the math functions in this file. This function should remain at the
// bottom of the file because it references all of the functions above.
void sqlite3xMathFunctions(void) {
  static FuncDef funcs[] = {
    FUNCTION(acos, 1, 0, 0, acosFunc),
    FUNCTION(asin, 1, 0, 0, asinFunc),
    FUNCTION(atan, 1, 0, 0, atanFunc),
    FUNCTION(atan2, 2, 0, 0, atan2Func),
    FUNCTION(ceil, 1, 0, 0, ceilFunc),
    FUNCTION(cos, 1, 0, 0, cosFunc),
    FUNCTION(cosh, 1, 0, 0, coshFunc),
    FUNCTION(e, 0, 0, 0, eFunc),
    FUNCTION(exp, 1, 0, 0, expFunc),
    FUNCTION(floor, 1, 0, 0, floorFunc),
    FUNCTION(log, 1, 0, 0, logFunc),
    FUNCTION(log10, 1, 0, 0, log10Func),
    FUNCTION(pi, 0, 0, 0, piFunc),
    FUNCTION(pow, 2, 0, 0, powFunc),
    FUNCTION(sin, 1, 0, 0, sinFunc),
    FUNCTION(sinh, 1, 0, 0, sinhFunc),
    FUNCTION(sqrt, 1, 0, 0, sqrtFunc),
    FUNCTION(tan, 1, 0, 0, tanFunc),
    FUNCTION(tanh, 1, 0, 0, tanhFunc),
  };
  sqlite3InsertBuiltinFuncs(funcs, ArraySize(funcs));
}
