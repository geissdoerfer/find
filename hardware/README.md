# FIND/FLYNC Hardware

This directory contains the design files for a 29mmx29mm battery-free sensor node.
The node is powered by 3 IXYS KXOB025-5X3F solar panels and buffers energy in a 47uF multi-layer ceramic capacitor.
It implements a circuit to extract a clock signal from variations of the solar panel current caused by powerline flicker of lamps.

Key specifications:
 - TI BQ25505 boost charger with maximum power point tracking
 - Nordic Semiconductor nRF52840 2.4GHz wireless MCU (BLE/802.15.4)
  - ARM Cortex-M4F 64MHz CPU with FPU
  - 1MB flash, 256kB S-RAM
 - 47uF Capacitor in 0805 package

<img src="proto_front.png" width="720">

## Pinout

We use a head-less 6-pin Tag-Connect header to break out 6 signals from the FLYNC node.
Use a [Tag-Connect TC2030-IDC-NL cable](https://www.tag-connect.com/product/tc2030-idc-nl) cable to connect to this header.

| Pin number | Signal   | Description                            |
|------------|----------|----------------------------------------|
| 1          | SWDIO    | ARM Serial Wire Debug I/O              |
| 2          | SWDCLK   | ARM Serial Wire Debug Clock            |
| 3          | PIN_DBG1 | nRF52 GPIO for debugging via e.g. UART |
| 4          | PIN_DBG2 | nRF52 GPIO for debugging via e.g. UART |
| 5          | VCC      | Output of BQ25504 and supply of nRF52  |
| 6          | GND      | Ground                                 |


<img src="connector.png" width="480">