# xkcd-alt

A CLI tool for printing the daily [XKCD](https://xkcd.com/) alt text one-liner.

```
Usage: xkcd-alt [-h] [-b[ ][BACK]] [-o] [-v] [-k]

Prints the alt text for the most recent XKCD comic.

Options:
  -h, --help          Print this usage and exit
  -V, --version       Print version information and exit

  -b[ ][BACK], --back[=][BACK]
                      Print alt text for the bth previous XKCD strip. If
                      not given a value, implicitly sets b=1.

  -o, --one-line      Print alt text and attestation on one line.
  -v, --verbose       Allow cURL to print what's going on to stderr.
                      Useful for debugging or satisfying curiosity.
  -k, --insecure      Allow cURL to skip verification of the server's SSL
                      certificate. Try not to specify this.
```

## Dependencies

[Boost](https://www.boost.org/) 1.71+ headers and
[libcurl](https://curl.se/libcurl/) 7.68+. libcurl is a runtime dependency
unless building with a static libcurl library.

[Boost.ProgramOptions](https://theboostcpplibraries.com/boost.program_options)
1.71+ is an optional compile-time and run-time dependency, required only if
during build time it is requested as the backing implementation for
command-line option parsing. The default implementation is hand-written, which
removes any runtime dependencies on Boost libraries.

Note that if Boost.ProgramOptions is used the usage printout will look a bit
different.

## Building from source

### *nix

Building is easy with the provided `build.sh` build script. For usage, type

```bash
./build.sh --help
```

To build release binaries for this project, simply use the command

```bash
./build.sh -c Release
```

Simply typing `./build.sh` will build unoptimized binaries with debug symbols.
Please note that if `pkg-config` is not installed, due to how the CMake
[`FindCURL`](https://cmake.org/cmake/help/latest/module/FindCURL.html) find
module is implemented, CMake will error out with:

```
CMake Error at /usr/share/cmake-3.22/Modules/FindCURL.cmake:175 (message):
  CURL: Required feature HTTPS is not found
Call Stack (most recent call first):
  CMakeLists.txt:77 (find_package)
```

Ensure that your system has `pkg-config` installed, i.e. with `apt`, etc.

> [!NOTE]
>
> This project provides its own libcurl find capability via a CMake macro,
> `pdxka_find_curl`, that can correctly determine the requested libcurl
> components without relying on the libcurl CMake config script (which as of
> 8.11.2 erroneously fails to set `CURL_FOUND` to true), pkg-config, or
> `curl-config` (which is unusable on Windows as it is a shell script). In the
> future, locating libcurl and its components may be done with
> `pdxka_find_curl` not just on Windows, in which case pkg-config is no longer
> necessary to locate libcurl.

### Windows

TBA. For now, here are some brief instructions for 64-bit builds.

To configure a 64-bit Windows build, the following CMake command can be used:

```shell
cmake -S . -B build_windows_x64 -A x64
```

To build the `pdxka` support library as a static library, specify
`-DBUILD_SHARED_LIBS=OFF` (on by default). Note that it may be necessary to
also specify `-DBoost_USE_STATIC_LIBS=OFF` if your Boost libraries are built as
DLLs as on Windows CMake will look for static libraries by default unless
instructed otherwise.

Then, one can build the Debug configuration using the following:

```shell
cmake --build build_windows_x64 --config Debug -j
```

Tests for the Debug config can be run with
[CTest](https://cmake.org/cmake/help/latest/manual/ctest.1.html), e.g. with

```shell
ctest --test-dir build_windows_x64 -C Debug -j20
```

## Gallery

```
Squaring the circle is really easy with some good clamps.
		-- https://xkcd.com/2706/
```

```
It's okay, they can figure out which control positions produce scalding water
via a trial-and-error feedback loop with a barely-perceptible 10-second lag.
		-- https://xkcd.com/2704/
```

```
Spacetime Soccer, known outside the United States as '4D Football' is a
now-defunct sport. Infamous for referee decisions hinging on inconsistent
definitions of simultaneity, it is also known for the disappearance of many top
players during... [more]
		-- https://xkcd.com/2705/
```

## CMake magic

A lot of CMake scripting has been done to get cross-platform builds working
correctly with CMake while also supporting source file generation and test
enumeration properly for single and multi-config build system generators. Here's
a quick bullet list of some arcane CMake scripting tasks:

* `pdxka_find_curl` to replace CMake's own `FindCURL` find module that supports
  `COMPONENTS` in a
  [`find_package`](https://cmake.org/cmake/help/latest/command/find_package.html)
  call, working around the libcurl CMake config script not exporting
  features + protocols
* `pdxka_boost_discover_tests` to discover and separately register all the tests
  in a Boost.Test test runner, like a simpler
  [`gtest_discover_tests`](https://cmake.org/cmake/help/latest/module/GoogleTest.html#command:gtest_discover_tests),
  by reading the output of `--list_content`
* `version.h` header file generation support for multi-config generators to
  run `configure_file` as a *pre-build* per-config generation step so each
  build config gets its own `version.h` generated header
* `pdxka_live_test.cmake` to implement CTest tests involving xkcd-alt making
  real network connections that can mark themselves as skipped if there are no
  running IPv4 or IPv6 network interfaces available

For the last bullet, this CMake script calls a program, `pdxka_inet_test`, to
perform the determination that the local machine is connected to the Internet.
On Windows the COM
[`INetworkListManager`](https://learn.microsoft.com/en-us/windows/win32/api/netlistmgr/nn-netlistmgr-inetworklistmanager)
is used, while for POSIX systems [`getifaddrs`](https://man7.org/linux/man-pages/man3/getifaddrs.3.html)
is used. Both implementations are wrapped in nice RAII C++ interfaces.
