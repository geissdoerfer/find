[![Build Status](https://travis-ci.com/geissdoerfer/find.svg?branch=master)](https://travis-ci.com/geissdoerfer/find)

# Fast neighbor discovery and synchronization in battery-free networks with FIND & FLYNC

This repository hosts the model, hardware and firmware complementing our NSDI 2021 paper.
FIND is the first neighbor discovery protocol for battery-free networks that uses randomized waiting to minimize discovery latency.
FLYNC is a new hardware/software solution for indoor light harvesting devices that exploits powerline-induced flicker of widely used lamps to further speed up discovery.

To get an overview, visit [our website](https://find.nes-lab.org).
For a detailed description and evaluation results, take a look at [our paper](https://nes-lab.org/pubs/2021-Geissdoerfer-Find.pdf).

## Model

We provide a Python implementation of the model underlying the proposed ND protocol.
For a detailed description and examples, see the corresponding [README](./model/README.md).

## Hardware

We've built a tiny, battery-free sensor node to evaluate FIND and FLYNC on real hardware.
The [firmware](./hardware) folder contains the corresponding schematics and layout files.

## Firmware

The C code in the [firmware](./firmware) directory runs the FIND protocol together with the FLYNC synchronization mechanism on the sensor node provided in `hardware`.


## People

FIND and FLYNC are developed at the Networked Embedded Systems Lab at TU Dresden as part of the DFG-funded project Next-IoT.

The following people have contributed to FIND and FLYNC:

 - [Kai Geissdoerfer](https://scholar.google.com/citations?user=k8YZfQEAAAAJ)
 - [Marco Zimmerling](https://wwwpub.zih.tu-dresden.de/~mzimmerl/)
