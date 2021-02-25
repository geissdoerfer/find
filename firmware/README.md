# FIND & FLYNC firmware

The C code in this directory runs the FIND protocol together with the FLYNC synchronization mechanism on the sensor node hardware provided in `../hardware`.

## Prerequisites

To get started, you'll need:

 - Two FLYNC sensor nodes
 - A [Tag-Connect TC2030-IDC-NL cable](https://www.tag-connect.com/product/tc2030-idc-nl)
 - A programmer compatible with SWD and the nRF52. For example, the [J-Link Mini Edu](https://www.segger.com/products/debug-probes/j-link/models/j-link-edu-mini/), the [Black Magic Probe](https://github.com/blacksphere/blackmagic/wiki) or a [nRF52-DK](https://www.nordicsemi.com/Software-and-Tools/Development-Kits/nRF52-DK).
 - A 3.3V power supply
 - A reasonably bright lamp with significant powerline flicker. All incandescent and fluorescent lamps as well as most cheap LED lamps should work.

## Getting started

### Building
 - Make sure you have the [GNU Arm Embedded Toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads) installed
 - If `arm-none-eabi-gcc` is not in your path, set the environment variable `GNU_INSTALL_ROOT` accordingly, e.g.: `export GNU_INSTALL_ROOT=/opt/toolchain/` (note the trailing foreslash)
 - Download the nRF5 SDK from [here.](https://www.nordicsemi.com/Software-and-tools/Software/nRF5-SDK/Download "here.") (You don't need a SoftDevice) and extract it
 - Set the environment variables `SDK_ROOT` to the corresponding absolute path, e.g.: `export SDK_ROOT=/home/user/nRF5_SDK_17.0.2_d674dde/`
 - run `make`

### Flashing
 - Download and install the [nRF-Command-Line-Tools](https://www.nordicsemi.com/Software-and-tools/Development-Tools/nRF-Command-Line-Tools/Download) following the official instructions.
 - Connect your programmer to one of the FLYNC sensor nodes according to the corresponding [README](../hardware/README.md).
 - run `make flash`

