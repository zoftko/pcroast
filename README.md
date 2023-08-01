# pcroast
Convert an electric oven into a modern reflow oven, controlled by a Raspberry Pico (RP2040).

## Project Layout
### `vendor`
Contains all third party code in the form of git submodules. This includes:

* pico-sdk: Provides libraries to write code for RP2040 processors
* free-rtos: Real time operating system to keep track of all the control tasks performed by the system

### `pico`
All RP2040 dependent code is included in this directory. Basically
all peripheral access. It also includes configuration for RTOS.

## Building
This is a standard CMake project and can be built as such. The project is built and tested with
ARM GCC. Logging macros depend on `__FILE_NAME__` so the compiler must define this macro. GCC
versions below 12.3 don't define it and won't be able to build the project.
