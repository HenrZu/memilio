# MEmilio C++ #

The MEmilio C++ library contains the implementation of the epidemiological models. 

Directory structure:
- memilio: framework for developing epidemiological models with, e.g., interregional mobility implementations, nonpharmaceutical interventions (NPIs), and  mathematical, programming, and IO utilities.
- models: implementation of concrete models (ODE and ABM)
- simulations: simulation applications that were used to generate the scenarios and data for publications
- examples: small applications that help with using the framework and models
- tests: unit tests for framework and models.
- cmake: build utility code
- thirdparty: configuration of dependencies

## Requirements

MEmilio C++ uses CMake (version 3.10 or higher) as a build system (https://cmake.org/).

MEmilio C++ is regularly tested with the following compilers (list will change over time):
- GCC, versions 7.3.0 - 10.2.0
- Clang, version 6.0 - 9.0
- MSVC, versions 19.16.27045.0 (Visual Studio 2017) - 19.29.30133.0 (Visual Studio 2019)

MEmilio C++ is regularly tested on gitlub runners using Ubuntu 18.04 and 20.04 and Windows Server 2016 and 2019. It is expected to run on any comparable Linux or Windows system. It is currently not tested on MacOS.

The following table lists the dependencies that are used. Most of them are required, but some are optional. The library can be used without them but with slightly reduced features. CMake will warn about them during configuration. The [Conan package manager](https://conan.io) is required to automatically aquire dependencies. Version compatibility needs to be ensured by the user, the version we currently use is included in the table.

| Library | Version  | Required       | Notes |
|---------|----------|----------------|-------|
| spdlog  | 1.5.0    | Yes            | https://github.com/gabime/spdlog |
| Eigen   | 3.3.9    | Yes            | http://gitlab.com/libeigen/eigen |
| Boost   | 1.75.0   | Yes            | https://www.boost.org/ |
| JsonCpp | 1.9.5    | No             | https://github.com/open-source-parsers/jsoncpp |
| HDF5    | 1.12.0   | No             | https://www.hdfgroup.org/, package libhdf5-dev on apt (Ubuntu) |
| GoogleTest | 1.10.0| For Tests only | https://github.com/google/googletest |

See the [thirdparty](thirdparty/README.md) directory for more details.

## Installation

### Configuring using CMake

If you don't have the Conan package manager installed yet, the recommended way is to install it using `pip` (a python virtual environment can be used, e.g., the same that you are using to install our python packages):
```bash
pip install conan
```
It is possible to build the project without Conan using the MEMILIO_USE_CONAN CMake variable, see below.

To configure with default options:
```bash
mkdir build && cd build
cmake ..
```
This also downloads dependencies using Conan. 

Options can be specified with `cmake .. -D<OPTION>=<VALUE>` or by editing the `build/CMakeCache.txt` file after running cmake. The following options are known to the library:
- `MEMILIO_BUILD_TESTS`: build unit tests in the test directory, ON or OFF, default ON.
- `MEMILIO_BUILD_EXAMPLES`: build the example applications in the examples directory, ON or OFF, default ON.
- `MEMILIO_BUILD_MODELS`: build the separate model libraries in the models directory, ON or OFF, default ON.
- `MEMILIO_BUILD_SIMULATIONS`: build the simulation applications in the simulations directory, ON or OFF, default ON.
- `MEMILIO_USE_CONAN`: use the Conan package manager to download dependencies, ON or OFF, default ON.
- `MEMILIO_USE_SYSTEM_<XYZ>`: use the dependency XYZ installed on the system instead of the Conan package, ON or OFF, default OFF.
- `MEMILIO_SANITIZE_ADDRESS/_UNDEFINED`: compile with specified sanitizers to check correctness, ON or OFF, default OFF.

Other important options may need:
- `CMAKE_BUILD_TYPE`: controls compiler optimizations and diagnostics, Debug, Release, or RelWithDebInfo; not available for Multi-Config CMake Generators like Visual Studio, set the build type in the IDE or when running the compiler.
- `CMAKE_INSTALL_PREFIX`: controls the location where the project will be installed
- `HDF5_DIR`: if you have HDF5 installed but it is not found by CMake (usually on the Windows OS), you may have to set this option to the directory in your installation that contains the `hdf5-config.cmake` file.

To e.g. configure the build without unit tests and with a specific version of HDF5:
```bash
cmake .. -DMEMILIO_BUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=Debug
```

### Making the library

After configuring, make the library using cmake:
```bash
cmake --build .
```

### Running the tests or examples

Run the unittests with:
```bash
./tests/memilio-test
```

Run an example with:
```
./examples/secir-example
```

### Installing

Install the project at the location given in the `CMAKE_INSTALL_PREFIX` variable with:
```bash
cmake --install .
```
This will install the libraries, headers, and executables that were built, i.e. where `MEMILIO_BUILD_<PART>=ON`.

### Using the libraries in your project

Using CMake, integration is simple. 

If you installed the project, there is a `memilio-config.cmake` file included with your installation. This config file will tell CMake which libraries and directores have to be included. Look up the config using the command `find_package(memilio)` in your own `CMakeLists.txt`. On Linux, the file should be found automatically if you installed in the normal GNU directories. Otherwise, or if you are working on Windows, you have to specify the `memilio_DIR` variable when running CMake to point it to the `memilio-config.cmake` file. Add the main framework as a dependency with the command `target_link_libraries(<your target> PRIVATE memilio::memilio)`. Other targets that are exported are `memilio::secir`, `memilio::seir`, and `memilio::abm`. This will set all required include directories and libraries, even transitive ones.

Alternatively, `MEmilio` can be integrated as a subdirectory of your project with `add_subdirectory(memilio/cpp)`, then you can use the same  `target_link_libraries` command as above.

## Known Issues

- Installing currently is not tested and probably does not work as expected or at all. If you want to integrate the project into yours, use the `add_subdirectory` way.
- On Windows, automatic detection of HDF5 installations does not work reliably. If you get HDF5 related errors during the build, you may have to supply the HDF5_DIR variable during CMake configuration, see above.