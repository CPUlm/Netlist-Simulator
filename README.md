<picture>
  <source media="(prefers-color-scheme: dark)" srcset="https://raw.githubusercontent.com/desfreng/netlist/hubert/logo/logo_light.svg">
  <source media="(prefers-color-scheme: light)" srcset="https://raw.githubusercontent.com/desfreng/netlist/hubert/logo/logo.svg">
  <img alt="The Netlist++ logo." src="https://raw.githubusercontent.com/desfreng/netlist/hubert/logo/logo.svg">
</picture>

# [Netlist++](https://github.com/hbens/sysnum-2023)

## How to build

This project uses [CMake](https://cmake.org/) as the build system.

Therefore, to compile the project, you can type the following commands:
```shell
mkdir build
cd build
cmake ../
cmake --build .
```

Of course, there are many other ways to do it. 
Furthermore, CMake is able to use Make files or Ninja files, etc. 

### The targets provided by the CMake file

- `netlist`: the main Netlist program
- `netlist_test`: the unit tests
- `netlist_doc`: the Doxygen documentation generation target

## Dependencies

The project uses C++20. Please use a modern C++ compiler. Something like Visual Studio 2019, GCC 13 or Clang 14 will work.
Others, older compilers, may also work.

### Hard dependencies

These are the dependencies needed to compile and run the program.

- [fmt](https://github.com/fmtlib/fmt): an incredible C++ formatting library. Even if C++20 support the same API, some guys on our team (and GitHub Actions) doesn't have access to modern compilers. Because it is a hard dependency, its source code is included in the directory as a git submodule and automatically compiled by the CMake file.

### Soft dependencies

These are the dependencies needed or recommended to develop the program.

- [gperf](https://www.gnu.org/software/gperf/): a perfect hash table generator used by the lexer to classify the keywords. If you aren't intent to add or remove keywords, this dependency is not required as the generated file is already provided. If installed, CMake will automatically use it.
- [doxygen](https://www.doxygen.nl/index.html): the well-known documentation generator for C++ programs. Only needed to generate the documentation. CMake will try to detect it automatically and provide the `netlist_doc` target to generate the HTML documentation.

## License

This project was written by Gabriel Desfrene, Hubert Gruniaux, Vincent Jules and Clément Rouvroy.

There is, for now, no specific license.
