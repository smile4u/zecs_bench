# zecs_bench

[![CI](https://github.com/smile4u/zecs_bench/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/smile4u/zecs_bench/actions/workflows/ci.yml)

A benchmark that compares [ZECS](https://github.com/smile4u/zecs) with
[EnTT](https://github.com/skypjack/entt).

## Build and run

You need a C++20 compiler and CMake 3.25 or newer.

```
git clone --recursive https://github.com/smile4u/zecs_bench.git
cmake -S zecs_bench -B build
cmake --build build
./build/zecs_bench > results.md
```

The report is markdown. It starts with the build and the machine, and it tells
what every table measures. A run takes about one minute and needs up to 3 GB of memory.

## Third-party code

- [EnTT](https://github.com/skypjack/entt) 4.0.0 in `ext/entt`, MIT license.
- [picobench](https://github.com/iboB/picobench) 2.11.0 in `ext/picobench`, MIT license.
  `picobench.patch` adds the median of the samples to its report.

## License

MIT, see [LICENSE](LICENSE).
