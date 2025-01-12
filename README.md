# xkcd-alt

A CLI tool for printing the daily [XKCD](https://xkcd.com/) alt text one-liner.

```
Usage: xkcd-alt [OPTION...]

Prints the alt text for the most recent XKCD comic.

General options:
  -b [ --back ] [=arg(=1)] (=0) Print alt-text for bth previous XKCD strip. If
                                not given a value, implicitly sets b=1.
  -o [ --one-line ]             Print alt text and attestation on one line.

Debug options:
  -v [ --verbose ]              Allow cURL to print what's going on to stderr.
                                Useful for debugging or satisfying curiosity.
  -k [ --insecure ]             Allow cURL to skip verification of the server's
                                SSL certificate. Try not to specify this.

Other options:
  -h [ --help ]                 Print this usage and exit
  -V [ --version ]              Print version information and exit
```

## Dependencies

[Boost](https://www.boost.org/) 1.71+ headers,
[Boost.ProgramOptions](https://theboostcpplibraries.com/boost.program_options)
1.71+, and [libcurl](https://curl.se/libcurl/) 7.68+.

In the future, the dependence on Boost.ProgramOptions may be removed.

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
