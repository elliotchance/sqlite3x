#ifndef SQLITE_DECIMAL_H
#define SQLITE_DECIMAL_H

#include "libbf.h"

typedef struct decimal_t {
  bfdec_t dec; // Contains the actual number.

  // The precision and scale are not stored within the "dec" itself and are
  // required for most operations. You can find full descriptions of what these
  // represent in sqlite3xDecimalGetPrecision() and sqlite3xDecimalGetScale()
  // respectively.
  limb_t precision, scale;
} decimal_t;

void sqlite3xDecimalOpCode(u8 opcode, Mem *out, Mem *in1, Mem *in2);

#endif /* SQLITE_DECIMAL_H */
