# [NetList Project](https://github.com/desfreng/netlist/tree/gabriel)

## How to build

To build this project, you will need a decent C++ compiler and [CMake](https://cmake.org/).
On a standard Linux machine, these commands should be fine:


```shell
mkdir "build"
cd "build"
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j
```

## Unit tests

Some tests have been added (more to come!) in the `unittest` folder. 
They check various parts of the program. 
To run them, use: `make test` in the build directory.

## Dependencies

This project uses some part of C++20. To compile, please use:

- GCC 12.2.0 or later
- Clang 14.0.6 or later
- MSVC 19.37.32825.0 or later (if you really want to compile on Windows...)

You can refer to the [workflows](https://github.com/desfreng/netlist/actions) to check if
your compiler is supported. 

No libraries are used except the C++ Standard Library.

## Remarks

Many thanks to [Hubert Gruniaux](https://github.com/hgruniaux) for laying the foundations
of the project : lexer, parser and errors messages.