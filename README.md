# pcroast
Convert an electric oven into a modern reflow oven, controlled by a Raspberry Pico (RP2040).

## Project Layout
### `vendor`
Contains all third party code in the form of git submodules. This includes:

* pico-sdk: Provides libraries to write code for RP2040 processors
* free-rtos: Real time operating system to keep track of all the control tasks performed by the system
* ugfx: Graphics library used for the display
* googletest: Used for unit testing

### `config`
Contains all configuration files for the application, e.g. `FreeRTOSConfig.h` or `lwipopts.h`.

### `pico`
All RP2040 dependent code is included in this directory. Basically
all peripheral access. It also includes configuration for RTOS.

### `src`
This is platform independent code, ideally all business logic should be found here. This makes it possible to build
and test said logic in targets other than the original RP2040 processor.

### `tests`
This folder contains all tests for business logic code units, found in `src`. Said tests and
the business logic tested are built using the host's compiler (generally amd64) and run in the host. These tests
are automatically run on CI pipelines.

## Building
This is a standard CMake project and can be built as such. The project is built and tested with
ARM GCC. Logging macros depend on `__FILE_NAME__` so the compiler must define this macro. GCC
versions below 12.3 don't define it and won't be able to build the project.

### Running Tests
The variable `HOST_BUILD` determines whether the project will be built for an RP2040 target (cross compiling),
or for the host's target (usually your dev machine). Tests are only run on host builds. The following snippet builds
and runs the tests:

```shell
cmake -DHOST_BUILD=YES -B cmake-build-host
make -C cmake-build-host
make -C cmake-build-host ARGS="-V" test
```
