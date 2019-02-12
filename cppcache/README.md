Contents
========


## Main sources

# include/
Public include header files for C++ library.

# src/
Sources for both static and shared C++ library.

# shared/
Shared library definition only. Should not contain any sources.

# static/
Static library definition only. Should not contain any sources.


## Test and Benchmark sources

# integration/
Integration syle tests, benchmarks, and common sources. These are all "modern"
single process style and shoult not use any of the "legacy" multiple process
framework.

## integrtion-test/
Legacy integration tests written in the multiple process framework. No new tests
should be added to this collections. Fixes and rewrites should be migrated to
the new framework in the _integration/test_ directory metioned above.

## test/
Unit style tests.

## benchmark/
Unit style or micro benchmark tests.

