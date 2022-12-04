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
1.71+, and [cURL](https://curl.se/) 7.68+.

## Building from source

TBA; for now see the "How to build" comments in the top-level `CMakeLists.txt`.

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
