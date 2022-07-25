# Simple JSON parser

This is a simple hand-made JSON parser for illustration purpose.

## Build

```
$ mkdir build && cd build
$ cmake ..
$ make
```

## Test

Run
```
$ for line in `cat ../json.txt`; do ./main <<< $line; echo; done
```
