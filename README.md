# sqlite3x [![Build Status](https://travis-ci.org/elliotchance/sqlite3x.svg?branch=master)](https://travis-ci.org/elliotchance/sqlite3x)

   * [Overview](#overview)
   * [Features](#features)
      * [Added Functions](#added-functions)
         * [acos(X)](#acosx)
         * [asin(X)](#asinx)
         * [atan(X)](#atanx)
         * [atan2(Y,X)](#atan2yx)
         * [ceil(X)](#ceilx)
         * [cos(X)](#cosx)
         * [cosh(X)](#coshx)
         * [e()](#e)
         * [exp(X)](#expx)
         * [floor(X)](#floorx)
         * [log(X)](#logx)
         * [log10(X)](#log10x)
         * [pi()](#pi)
         * [pow(X,Y)](#powxy)
         * [sin(X)](#sinx)
         * [sinh(X)](#sinhx)
         * [sqrt(X)](#sqrtx)
         * [tan(X)](#tanx)
         * [tanh(X)](#tanhx)
   * [Building From Source](#building-from-source)
      * [Binary Client](#binary-client)
      * [Amalgamation Source](#amalgamation-source)
   * [Testing](#testing)
   * [Updating SQLite3 Core](#updating-sqlite3-core)


Overview
========

sqlite3x is a fork of the sqlite3 codebase that provides 100% compatibility with
sqlite3. Data read or modified in either sqlite3 ot sqlite3x is always
compatible with the other one.

sqlite3x is kept up to date with the latest sqlite3 codebase and contains
modifications that sit on top of the engine, such as new functions.

The only compatibility issues will be that if you use sqlite3x functions, they
will obviously not be available for use in queries on a vanilla sqlite3 command
line or linked library.


Features
========

Arbitrary Precision (Decimal) Arithmetic
----------------------------------------

Arbitrary precision arithmetic (sometimes called decimal math) is important when
performing operations on exact values, where approximations and rounding errors
from floating-point values are not permitted. One good example of this is
calculations that involve money.

sqlite3x handles arbitrary precision arithmetic automatically. Internally, the
execution engine (also called VDBE) has been modified to offload operations
between `TEXT` to the [LibBF library](https://bellard.org/libbf/) created by
Fabrice Bellard.

Here is an example of how sqlite3 and sqlite3x handle the same operation:

```sqlite
SELECT "12345678901234567890.123" + "234.5678901234567890234567890123456789";

-- sqlite3 result: 1.23456789012346e+19
-- sqlite3x result: 12345678901234568124.6908901234567890234567890123456789
```

### Important Notes

1. Arbitrary precision arithmetic will only be used when all inputs are `TEXT`. If
you mix types (`1.23 + "4.56"`, for example) the normal approximation will be
used and an int or floating-point value will be returned.

2. The [type affinity](https://www.sqlite.org/datatype3.html#type_affinity) of
the column must be `TEXT`:

`TEXT` affinity must have one of the following types (the size specified is
ignored): `CHARACTER(20)`, `VARCHAR(255)`, `VARYING CHARACTER(255)`,
`NCHAR(55)`, `NATIVE CHARACTER(70)`, `NVARCHAR(100)`, `TEXT`, `CLOB`.

The following affinities will *not* use decimal arithmetic (notice that SQL
`NUMERIC` and `DECIMAL` will not have a `TEXT` affinity, so they cannot be
used):

`INTEGER` affinity: `INT`, `INTEGER`, `TINYINT`, `SMALLINT`, `MEDIUMINT`,
`BIGINT`, `UNSIGNED BIG INT`, `INT2`, `INT8`.

`BLOB` affinity: `BLOB` or no data type specified.

`REAL` affinity: `REAL`, `DOUBLE`, `DOUBLE PRECISION`, `FLOAT`.

`NUMERIC` affinity: `NUMERIC`, `DECIMAL(10,5)`, `BOOLEAN`, `DATE`, `DATETIME`.

3. Decimal values must be well formed (in the form of "###" or "###.###").
Values that are not well formed may produce unexpected results and are not
guaranteed to return an error.

4. `NULL` rules still apply to decimal values. That is, a `NULL` value used in
arithmetic will always return a `NULL` result.


Decimal Functions
-----------------

### precision(X)

`precision()` is only designed to work with text values that represents a well
formed exact number. Attempting to use it with a non well formed value will
produce unexpected results.

The precision is the minimum number of digits (not including the decimal point,
or negative sign) required to accurately represent the number. For example:

```sql
precision("0")         -- 1
precision("0.0")       -- 1
precision("123")       -- 3
precision("-123")      -- 3
precision("1230")      -- 4
precision("01230")     -- 4
precision("123.0")     -- 3
precision("123.45")    -- 5
precision("-123.45")   -- 5
precision("0123.4500") -- 5
```

For any other argument type it will return `NULL`. Should you need to use
`precision()` on a non-text value you must cast it first. However, keep in mind
that floating-point values are intrinsically approximations so the result of
`precision()` cannot always be trusted in those cases.

## scale(X)

`scale()` is only designed to work with text values that represents a well
formed exact number. Attempting to use it with a non well formed value will
produce unexpected results.

The scale is the number of digits after the decimal point, excluding trailing
zeros:

```sql
scale("0")         -- 0
scale("0.0")       -- 0
scale("123")       -- 0
scale("-123")      -- 0
scale("1230")      -- 0
scale("01230")     -- 0
scale("123.0")     -- 0
scale("123.45")    -- 2
scale("-123.45")   -- 2
scale("0123.4500") -- 2
```

For any other argument type it will return `NULL`. Should you need to use
`scale()` on a non-text value you must cast it first. However, keep in mind that
floating-point values are intrinsically approximations so the result of
`scale()` cannot always be trusted in those cases.


Math Functions
--------------

On top of the [sqlite3 built-in functions](
https://www.sqlite.org/lang_corefunc.html), sqlite3x adds:

### acos(X)

acos(X) returns the arc-cosine of *X*. If *X* is `NULL`, then the result will be
`NULL`.

If *X* not numeric then it will be treated internally as `0.0`. This will
produce a result of `1.5707963267949`.

### asin(X)

asin(X) returns the arc-sine of *X*. If *X* is `NULL`, then the result will be
`NULL`.

If *X* not numeric then it will be treated internally as `0.0`. This will
produce a result of `0.0`.

### atan(X)

atan(X) returns the arc-tangent of *X*. If *X* is `NULL`, then the result will
be `NULL`.

If *X* not numeric then it will be treated internally as `0.0`. This will
produce a result of `0.0`.~~~~

### atan2(Y,X)

atan2(Y,X) returns arc-tangent with *Y* and *X*. `NULL` is returned if either of
the inputs are `NULL`.

You should be careful that inputs are valid and sensible. atan2(Y,X) does not
provide any protection for values that are invalid (not valid numbers).

### ceil(X)

ceil(X) rounds *X* upward, returning the smallest integral value that is not
less than *X*. If *X* is `NULL`, then the result will be `NULL`, otherwise the
result will always be a floating-point value.

If *X* not numeric then it will be treated internally as `0.0`. This will
produce a result of `0.0`.

### cos(X)

cos(X) returns the cosine of *X* (*X* is expressed in radians). If *X* is
`NULL`, then the result will be `NULL`.

If *X* not numeric then it will be treated internally as `0.0`. This will
produce a result of `1.0`.

### cosh(X)

cosh(X) returns the hyperbolic cosine of *X*. If *X* is `NULL`, then the result
will be `NULL`.

If *X* not numeric then it will be treated internally as `0.0`. This will
produce a result of `0.0`.

### e()

e() returns the value of e, which is approximately 2.71828.

### exp(X)

exp(X) returns *e* to the power of *X*. If *X* is `NULL`, then the result will
be `NULL`.

If *X* not numeric then it will be treated internally as `0.0`. This will
produce a result of `1.0`.

exp(X) does not provide any protection for result values that are too small or
large (overflow) will have unexpected behavior depending on your system.

### floor(X)

floor(X) rounds *X* downward, returning the largest integral value that is not
greater than *X*. If *X* is `NULL`, then the result will be `NULL`, otherwise
the result will always be a floating-point value.

If *X* not numeric then it will be treated internally as `0.0`. This will
produce a result of `0.0`.

### log(X)

log(X) returns natural logarithm (base *e*) of *X*. If *X* is `NULL`, then the
result will be `NULL`.

If *X* not numeric then it will be treated internally as `0.0`. This will
produce a result of `-Inf`.

### log10(X)

log10(X) returns common logarithm (base 10) of *X*. If *X* is `NULL`, then the
result will be `NULL`.

If *X* not numeric then it will be treated internally as `0.0`. This will
produce a result of `-Inf`.

### pi()

pi() returns the value of pi, which is approximately 3.14159.

### pow(X,Y)

pow(X,Y) returns *X* to the power of *Y*. The result is always a floating-point
value. `NULL` is returned if either of the inputs are `NULL`.

You should be careful that inputs are valid and sensible. pow(X,Y) does not
provide any protection for values that are invalid (not valid numbers). Result
values that are too small or large (overflow) will have unexpected behavior
depending on your system.

### sin(X)

sin(X) returns the sine of *X* (*X* is expressed in radians). If *X* is `NULL`,
then the result will be `NULL`.

If *X* not numeric then it will be treated internally as `0.0`. This will
produce a result of `0.0`.

### sinh(X)

sinh(X) returns the hyperbolic sine of *X*. If *X* is `NULL`, then the result
will be `NULL`.

If *X* not numeric then it will be treated internally as `0.0`. This will
produce a result of `0.0`.

### sqrt(X)

sqrt(X) returns the square root of *X*. If *X* is `NULL`, then the result will
be `NULL`.

If *X* not numeric then it will be treated internally as `0.0`. This will
produce a result of `0.0`.

### tan(X)

tan(X) returns the tangent of *X* (*X* is expressed in radians). If *X* is
`NULL`, then the result will be `NULL`.

If *X* not numeric then it will be treated internally as `0.0`. This will
produce a result of `0.0`.

### tanh(X)

tanh(X) returns the hyperbolic tangent of *X*. If *X* is `NULL`, then the result
will be `NULL`.

If *X* not numeric then it will be treated internally as `0.0`. This will
produce a result of `0.0`.


Building From Source
====================

Since sqlite3x inherits all of the source code from sqlite3, it also inherits
the build and test system that has been designed to work with many operating
systems.

Even though the process is the same (all of the documentation on sqlite.org can
be used), keep in mind that extra targets have been added with an "x" on the
end. This is so source and binaries that are distributed will not get confused
or replace any vanilla version of sqlite3.

You can clone this repository, or download a [zip of the latest stable
code](https://github.com/elliotchance/sqlite3x/archive/master.zip).

Binary Client
-------------

The binary client is the interactive shell you can use in your terminal to
operate sqlite3 databases. It can be built and updated with:

```bash
./configure
make sqlite3x       # For macOS and linux.
make sqlite3x.exe   # For windows.

# You may want to move the binary into $PATH so it can be used anywhere.
```

Amalgamation Source
-------------------

The amalgamation source is the most recommended way to integrate with your
existing source/application. Use make to produce two files that can be included
in your project:

```bash
./configure
make sqlite3x.c sqlite3x.h
```

Testing
=======

Running the full test suite is recommended before integrating sqlite3x into your
application.

The full test suite (including TCL tests for both core and sqlite3x) can be run
with:

```bash
make test
```

It can take some time to run the full suite. If you want to run a specific test
you can use:

```bash
make testfixture && ./testfixture test/main.test
```

Updating SQLite3 Core
=====================

As new versions of sqlite3 are released, the core source code can be updated
with:

```bash
fossil clone https://www.sqlite.org/src sqlite.fossil
fossil open sqlite.fossil
fossil update release
```
