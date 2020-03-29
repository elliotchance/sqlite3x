# sqlite3x

100% compatible sqlite3 fork with more features.

Updating SQLite3 Core Code
--------------------------

As new version of sqlite3 are released, the core source code can be updated
with:

```bash
fossil clone https://www.sqlite.org/src sqlite.fossil
fossil open sqlite.fossil
fossil update release
```

Building SQLite3x
-----------------

To produce the binary that is the CLI:

```bash
./configure
make sqlite3x     # macOS and linux
make sqlite3x.exe # windows
```

Testing
-------

Run all tests with:

```bash
make test
```
