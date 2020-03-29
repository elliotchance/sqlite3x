# sqlite3x

sqlite3x is a fork of the sqlite3 codebase that provides 100% compatibility with
sqlite3. Data read or modified in either sqlite3 ot sqlite3x is always
compatible with the other one.

sqlite3x is kept up to date with the latest sqlite3 codebase and contains
modifications that sit on top of the engine, such as new functions.

The only compatibility issues will be that if you use sqlite3x functions, they
will obviously not be available for use in queries on a vanilla sqlite3 command
line or linked library.


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
# Only needs to be run once to build the binary.
make testfixture

# Execute testfixture with a list of test files.
./testfixture test/main.test
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
