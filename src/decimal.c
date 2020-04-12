// This file is not part of the core sqlite3 codebase. It is part of the
// github.com/elliotchance/sqlite3x project.
//
// This file provides arbitrary-precision math functions that are implemented in
// libbf.

#include <string.h>
#include <math.h>
#include "sqliteInt.h"
#include "libbf.h"

// sqlite3xDecimalRealloc is a proxy for the built-in realloc(), and is used by
// the bf library internally.
static void *sqlite3xDecimalRealloc(void *opaque, void *ptr, size_t size) {
  return sqlite3Realloc(ptr, size);
}

// sqlite3xDecimalNewContext creates a new context which handles the allocation
// and processing of operations.
bf_context_t *sqlite3xDecimalNewContext() {
  bf_context_t *ctx = (bf_context_t *) sqlite3Malloc(sizeof(bf_context_t));
  bf_context_init(ctx, sqlite3xDecimalRealloc, NULL);

  return ctx;
}

// sqlite3xDecimalZero returns a decimal with the value of zero.
decimal_t *sqlite3xDecimalZero(bf_context_t *ctx) {
  decimal_t *x = (decimal_t *) sqlite3Malloc(sizeof(decimal_t));
  x->precision = 0;
  x->scale = 0;
  bfdec_init(ctx, &x->dec);

  return x;
}

// sqlite3xDecimalSetNaN transforms an existing decimal into a NaN value.
void sqlite3xDecimalSetNaN(decimal_t *x) {
  x->precision = 0;
  x->scale = 0;
  bfdec_set_nan(&x->dec);
}

// sqlite3xDecimalGetPrecision is used to fetch the precision from a string. It
// will assume that the string represents a well formed integer or decimal
// number. If the input it invalid the output will be unexpected behaviour.
//
// The minimum precision is 1, since even "0" has at least one digit.
//
// sqlite3xDecimalGetPrecision is used directly by the SQL function precision().
// See that for more information.
limb_t sqlite3xDecimalGetPrecision(const char *str) {
  // Before we begin, we must trim off the optional negative sign and any
  // leading zeros. This applies to both integers and decimals.
  char *p = (char *) str;
  while (*p == '-' || *p == '0') {
    p++;
  }

  // All solutions below will need this in one way or another, sometimes more
  // than once.
  int plen = strlen(p);

  // If the number does not contain a '.' it must be an integer. We can just the
  // total number of digits at this point.
  if (strchr(p, '.') == NULL) {
    int precision = plen;

    // digits will be 0 if all the digits are "0" (they will all be removed from
    // the previous step). Make sure we always return at least 1.
    if (precision < 1) {
      precision = 1;
    }

    return precision;
  }

  // Otherwise the value must be decimal. We need to trim any trailing zeros off
  // the end. We should not remove the trailing '.' (if any) because the
  // strlen() below will compensate as if it always exists (as we proved from
  // the if statement above).
  for (int i = plen - 1; i > 0 && p[i] == '0'; i--) {
    p[i] = '\0';
  }

  // Do not use plen beyond this point because p may have changed. The "-1" is
  // to remove the digit for the '.'.
  int precision = strlen(p) - 1;

  // Like the edge case above for integers, if the number only consists of zeros
  // on either size of the decimal then all the zeroes will be trimmed off,
  // producing a precision of 0 which is not correct.
  if (precision < 1) {
    precision = 1;
  }

  return precision;
}

// sqlite3xDecimalGetScale is used to fetch the scale from a string. It will
// assume that the string represents a well formed integer or decimal number. If
// the input it invalid the output will be unexpected behaviour.
//
// sqlite3xDecimalGetScale is used directly by the SQL function scale(). See
// that for more information.
limb_t sqlite3xDecimalGetScale(const char *str) {
  // If the number does not contain a '.' it must be an integer. So the scale
  // must be 0.
  char *p = strchr(str, '.');
  if (p == NULL) {
    return 0;
  }

  // Otherwise the value must be decimal. We need to trim any trailing zeros off
  // the end. We should not remove the trailing '.' (if any) because the
  // strlen() below will compensate as if it always exists (as we proved from
  // the if statement above).
  for (int i = strlen(p) - 1; i > 0 && p[i] == '0'; i--) {
    p[i] = '\0';
  }

  // The scale will be whatever is remaining between the '.' and the end of the
  // string (subtract 1 to compensate for the '.'). This is why we don't care
  // about removing any leading zeros, or the negative sign from the original
  // number.
  return strlen(p) - 1;
}

// sqlite3xDecimalNew creates a new decimal value from a string. It is assumed
// that the input is valid, if not the it will return NaN.
decimal_t *sqlite3xDecimalNew(bf_context_t *ctx, const char *str) {
  decimal_t *x = sqlite3xDecimalZero(ctx);
  x->precision = sqlite3xDecimalGetPrecision(str);
  x->scale = sqlite3xDecimalGetScale(str);

  int rc = bfdec_atof(&x->dec, str, NULL, x->precision, BF_RNDN);
  if (rc != 0) {
    sqlite3xDecimalSetNaN(x);
  }

  return x;
}

// sqlite3xDecimalDelete frees all memory associated with a decimal.
void sqlite3xDecimalDelete(decimal_t *x) {
  bfdec_delete(&x->dec);
  sqlite3_free(x);
}

// sqlite3xDecimalMaxPrecision returns the largest precision of the inputs.
limb_t sqlite3xDecimalMaxPrecision(limb_t a, limb_t b) {
  if (a > b) {
    return a;
  }

  return b;
}

// sqlite3xDecimalToString returns a new null-terminated string representation
// of the number.
char *sqlite3xDecimalToString(decimal_t *x) {
  // This is not used, but it must be provided to bfdec_ftoa().
  size_t digits;

  return bfdec_ftoa(&digits, &x->dec, x->precision, BF_RNDN);
}

// sqlite3xVdbeMemSetDecimalStr follows the conventions of other core
// "sqlite3VdbeMemSet*" functions to make it easier to set decimal values as
// results.
int sqlite3xVdbeMemSetDecimalStr(Mem *mem, decimal_t *x) {
  char *str = sqlite3xDecimalToString(x);

  int r = sqlite3VdbeMemSetStr(mem, str, -1, SQLITE_UTF8, SQLITE_TRANSIENT);
  sqlite3_free(str);

  return r;
}

// sqlite3xDecimalNormalize adjusts the precision and scale to accurately
// represent the decimal without any trailing zeros.
//
// First an explanation of how LibBF stores decimal values. All digits are
// stored in binary, but as base 10 numerically in the "tabs" property. Each tab
// contains up to 19 digits, and the tabs are in reverse. Here is how a large
// number is encoded:
//
//   12345678901234568124.6908901234567890234567890123456789000
//   \-----------------/\ -----------------/\-----------------/
//          tab[2]            tab[1]               tab[0]
//
// Using the expn = 20 we can calculate the real precision and scale:
//
// 1. Cleanly divide the first tab (tab[0]) by 10 repeatedly until the operation
// fails:
//
//   4567890123456789000 (i = 19)
//   456789012345678900 (i = 18)
//   45678901234567890 (i = 17)
//   4567890123456789 (i = 16)
//
// 2. Calculate the full precision:
//
//         len = 3
//   precision = ((len - 1) * 19) + i
//             = 54
//
// 3. Calculate the scale:
//
//   scale = precision - expn
//         = 34
//
// We have to be careful about how integers that are divisible by powers of 10
// are encoded. For example:
//
//   1234567890123456789012300000000.
//   \-----------------/\----------- ------/
//          tab[1]            tab[0]
//
// The expn is now 31. However, if we proceed with the method above:
//
//   0123000000000000000 (i = 19)
//   0123 (i = 4)
//
// 2. Calculate the full precision:
//
//         len = 2
//   precision = ((len - 1) * 19) + i
//             = 23
//
// 23 is not correct because it has removed the zeros that would be part of the
// integer. Since precision < expn (23 < 31) we know this must be a integer and
// so we can set:
//
//   precision = expn
//       scale = 0
//
void sqlite3xDecimalNormalize(decimal_t *x) {
  // Zero tabs just means the value is zero. Nothing to do.
  int tabLen = x->dec.len;
  if (x->dec.len == 0) {
    return;
  }

  // Step 1: Reduce i as much as possible.
  limb_t tab = x->dec.tab[0];
  int i = 19;
  for (; i > 0; i--) {
    if (tab % 10 != 0) {
      break;
    }

    tab /= 10;
  }

  // Step 2 & 3: Calculate the the precision and scale.
  x->precision = ((tabLen - 1) * 19) + i;

  // expn will be greater than precision for numbers that are clean multiples of
  // 10, such as "1680". See full description above.
  if (x->dec.expn > x->precision) {
    x->precision = x->dec.expn;
    x->scale = 0;
  } else {
    x->scale = x->precision - x->dec.expn;
  }
}

// sqlite3xDecimalAddOrSubtract will perform an addition or subtraction and
// return the result based on the provided "fn". These two operations have been
// collapsed into a single function because they do basically the same thing and
// work out their requirements the same way.
//
// This function guaranteed to return a decimal result. If an error occurs the
// result will be a NaN. See sqlite3xDecimalSetNaN().
//
// The result of this operation has well known expected outcomes for precision
// and scale. For example:
//
//   A = 123.456 (precision = 6, scale = 3)
//   B = 2456.7 (precision = 5, scale = 1)
//   A + B = 2580.156 (precision = 7, scale = 3)
//
// The result scale cannot be greater than either of the inputs, so:
//
//   scale = MAX(scaleA, scaleB)
//
// The result precision would have to be large enough to contain the largest
// integer, plus the highest scale, plus overflow expansion, so:
//
//   precision = MAX(precisionA - scaleA, precisionB - scaleB)  -- whole
//             + MAX(scaleA, scaleB)                            -- fraction
//             + 1                                              -- overflow
//
// The overflow expansion is required if the result adds together values of
// the a similar magnitude that would cause one extra precision to be required,
// for example:
//
//   53 (precision = 2) + 49 (precision = 2) = 102 (precision = 3)
//
// The precision and scale may have to be adjusted after the result to correctly
// represent the result. See sqlite3xDecimalNormalize() for full description.
decimal_t *
sqlite3xDecimalAddOrSubtract(bf_context_t *ctx, decimal_t *a, decimal_t *b,
                             int (*fn)(bfdec_t *r, const bfdec_t *a,
                                       const bfdec_t *b, limb_t prec,
                                       bf_flags_t flags)) {
  decimal_t *result = sqlite3xDecimalZero(ctx);
  result->scale = sqlite3xDecimalMaxPrecision(a->scale, b->scale);
  result->precision = sqlite3xDecimalMaxPrecision(
    a->precision - a->scale,
    b->precision - b->scale
  ) + result->scale + 1;

  int rc = fn(&result->dec, &a->dec, &b->dec, result->precision, BF_RNDN);
  if (rc != 0) {
    sqlite3xDecimalSetNaN(result);
  } else {
    sqlite3xDecimalNormalize(result);
  }

  return result;
}

// sqlite3xDecimalMultiply will multiply two decimal values and return the
// result.
//
// This function guaranteed to return a decimal result. If an error occurs the
// result will be a NaN. See sqlite3xDecimalSetNaN().
//
// The result of this operation has well known expected outcomes for precision
// and scale. For example:
//
//   A = 12.3 (precision = 3, scale = 1)
//   B = 4.567 (precision = 4, scale = 3)
//   A * B = 56.1741 (precision = 6, scale = 4)
//
// That is, the precision cannot be greater than the sum of the other two
// precisions. The scale cannot also be greater than the sum of the other two
// scales.
decimal_t *
sqlite3xDecimalMultiply(bf_context_t *ctx, decimal_t *a, decimal_t *b) {
  decimal_t *result = sqlite3xDecimalZero(ctx);
  result->scale = a->scale + b->scale;
  result->precision = a->precision + b->precision;

  int rc = bfdec_mul(&result->dec, &a->dec, &b->dec, result->precision, BF_RNDN);
  if (rc != 0) {
    sqlite3xDecimalSetNaN(result);
  } else {
    sqlite3xDecimalNormalize(result);
  }

  return result;
}

// sqlite3xDecimalAdd performs addition between two decimals and returns the
// result. See sqlite3xDecimalAddOrSubtract for full explanation.
decimal_t *sqlite3xDecimalAdd(bf_context_t *ctx, decimal_t *a, decimal_t *b) {
  return sqlite3xDecimalAddOrSubtract(ctx, a, b, bfdec_add);
}

// sqlite3xDecimalSub performs subtraction between two decimals and returns the
// result. See sqlite3xDecimalAddOrSubtract for full explanation.
decimal_t *
sqlite3xDecimalSubtract(bf_context_t *ctx, decimal_t *a, decimal_t *b) {
  return sqlite3xDecimalAddOrSubtract(ctx, a, b, bfdec_sub);
}

// sqlite3xDecimalDeleteContext frees up any memory associated with the context.
// This will not free any decimals created against the context, they will have
// to also be freed individually with sqlite3xDecimalDelete().
void sqlite3xDecimalDeleteContext(bf_context_t *ctx) {
  bf_context_end(ctx);
  sqlite3_free(ctx);
}

// sqlite3xDecimalOpCode performs an opcode (such as add, subtract, etc) and
// sets the result in the memory register.
void sqlite3xDecimalOpCode(u8 opcode, Mem *out, Mem *in1, Mem *in2) {
  bf_context_t *ctx = sqlite3xDecimalNewContext();
  decimal_t *a = sqlite3xDecimalNew(ctx, in2->z);
  decimal_t *b = sqlite3xDecimalNew(ctx, in1->z);

  decimal_t *r;
  switch (opcode) {
    case OP_Add:
      r = sqlite3xDecimalAdd(ctx, a, b);
      break;

    case OP_Subtract:
      r = sqlite3xDecimalSubtract(ctx, a, b);
      break;

    case OP_Multiply:
      r = sqlite3xDecimalMultiply(ctx, a, b);
      break;
  }

  sqlite3xVdbeMemSetDecimalStr(out, r);

  sqlite3xDecimalDelete(a);
  sqlite3xDecimalDelete(b);
  sqlite3xDecimalDelete(r);
  sqlite3xDecimalDeleteContext(ctx);
}

// precisionFunc implements the SQL function precision().
//
// precision() is only designed to work with text values that represents a well
// formed exact number. Attempting to use it with a non well formed value will
// produce unexpected results.
//
// The precision is the minimum number of digits (not including the decimal
// point, or negative sign) required to accurately represent the number. For
// example:
//
//   precision("0")         -- 1
//   precision("0.0")       -- 1
//   precision("123")       -- 3
//   precision("-123")      -- 3
//   precision("1230")      -- 4
//   precision("01230")     -- 4
//   precision("123.0")     -- 3
//   precision("123.45")    -- 5
//   precision("-123.45")   -- 5
//   precision("0123.4500") -- 5
//
// For any other argument type it will return NULL. Should you need to use
// precision() on a non-text value you must cast it first. However, keep in mind
// that floating-point values are intrinsically approximations so the result of
// precision() cannot always be trusted in those cases.
static void
precisionFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  assert(argc == 1);
  UNUSED_PARAMETER(argc);

  if (sqlite3_value_type(argv[0]) != SQLITE_TEXT) {
    sqlite3_result_null(context);
    return;
  }

  const char *x = (const char *) sqlite3_value_text(argv[0]);
  sqlite3_result_int(context, (int) sqlite3xDecimalGetPrecision(x));
}

// scaleFunc implements the SQL function scale().
//
// scale() is only designed to work with text values that represents a well
// formed exact number. Attempting to use it with a non well formed value will
// produce unexpected results.
//
// The scale is the number of digits after the decimal point, excluding trailing
// zeros:
//
//   scale("0")         -- 0
//   scale("0.0")       -- 0
//   scale("123")       -- 0
//   scale("-123")      -- 0
//   scale("1230")      -- 0
//   scale("01230")     -- 0
//   scale("123.0")     -- 0
//   scale("123.45")    -- 2
//   scale("-123.45")   -- 2
//   scale("0123.4500") -- 2
//
// For any other argument type it will return NULL. Should you need to use
// scale() on a non-text value you must cast it first. However, keep in mind
// that floating-point values are intrinsically approximations so the result of
// scale() cannot always be trusted in those cases.
static void
scaleFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  assert(argc == 1);
  UNUSED_PARAMETER(argc);

  if (sqlite3_value_type(argv[0]) != SQLITE_TEXT) {
    sqlite3_result_null(context);
    return;
  }

  const char *x = (const char *) sqlite3_value_text(argv[0]);
  sqlite3_result_int(context, (int) sqlite3xDecimalGetScale(x));
}

// Register the decimal functions in this file. This function should remain at
// the bottom of the file because it references all of the functions above.
void sqlite3xDecimalFunctions(void) {
  static FuncDef funcs[] = {
    FUNCTION(precision, 1, 0, 0, precisionFunc),
    FUNCTION(scale, 1, 0, 0, scaleFunc),
  };
  sqlite3InsertBuiltinFuncs(funcs, ArraySize(funcs));
}
