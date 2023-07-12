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
