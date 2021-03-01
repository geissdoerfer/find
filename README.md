[![Build Status](https://travis-ci.com/geissdoerfer/find.svg?branch=master)](https://travis-ci.com/geissdoerfer/find)

# Efficient neighbor discovery and synchronization in battery-free wireless networks with FIND and FLYNC

This repository hosts the artifacts accompanying our NSDI 2021 paper, which introduces FIND and FLYNC. FIND is the first neighbor discovery protocol for battery-free networks; it uses randomized waiting to minimize discovery latency in the face of intermittency. FLYNC synchronizes indoor light harvesting devices to the powerline-induced brightness variations of state-of-the-art lamps; it further speeds up discovery when used in tandem with FIND.

In addition to the model underlying FIND's optimized waiting time distribution, we provide the hardware design of the prototype battery-free node and the source code of the firmware we used to evaluate Find and FLYNC.

To get an overview, visit [our website](https://find.nes-lab.org).
For a detailed description and evaluation results, take a look at [our paper](https://nes-lab.org/pubs/2021-Geissdoerfer-Find.pdf).

## FIND Python Model

We provide a Python implementation of the analytical model of the FIND protocol, allowing to reproduce key results of our paper.
For a detailed description and examples, check out the [`/model`](./model/) directory.

## Hardware Design

We built a tiny, battery-free wireless node to evaluate FIND and FLYNC on real hardware.
The [`/hardware`](./hardware) directory contains the corresponding schematics and layout files.

## Firmware

The C code in the [`/firmware`](./firmware) directory implements the FIND protocol and the FLYNC synchronization mechanism on our prototype battery-free node.


## People

FIND and FLYNC are being developed at the Networked Embedded Systems Lab at TU Dresden.

The main contributors of FIND and FLYNC are:

 - [Kai Geissdoerfer](https://scholar.google.com/citations?user=k8YZfQEAAAAJ)
 - [Marco Zimmerling](https://wwwpub.zih.tu-dresden.de/~mzimmerl/)
