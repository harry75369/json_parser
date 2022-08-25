# Simple JSON parser

This is a simple hand-made JSON parser for illustration purpose.

## Build

```
$ mkdir build && cd build
$ cmake ..
$ make
```

## Test

In `build` folder, run
```
$ for line in `cat ../json.txt`; do ./main <<< $line; echo; done
```

## Performance Test

In `build` folder, run
```
$ for i in ../data/*.json; do echo $i; time ./main < $i > /dev/null; done
```

Test data are from: https://github.com/miloyip/nativejson-benchmark
