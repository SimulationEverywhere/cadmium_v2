# Cadmium 2
Object-Oriented implementation of a PDEVS simulator for C++ 17.
Cadmium 2 is a header-only library for implementing and simulating PDEVS, classic Cell-DEVS, and asymmetric Cell-DEVS models using an object-oriented programming paradigm.

NOTE: We are still working on this new version of Cadmium. We will shortly release a first official version with better documentation and more features.

## Requirements
Before using Cadmium, make sure that your computer has the following dependencies intalled:
- A C++ 17-compliant compiler (e.g., `g++` or `gcc`)
- `make`
- `cmake` 3.19 or greater
- `git`

## Getting started
- Download this repository:
```shell
git clone https://github.com/SimulationEverywhere/cadmium_v2.git
cd cadmium_v2
```
- Use the `build.sh` script to compile all the examples:
```shell
source build.sh
```
- You can execute the provided Cell-DEVS examples to ensure that everything is fine:
```shell
source celldevs_examples.sh
```
The files `example/celldevs_asymm_sir/log.csv` and `example/celldevs_grid_sir/log.csv` will contain the simulation traces of a pandemic scenario. Feel free to use your favorite tool (e.g., Microsof Excel, R, or Python with Pandas) to analyze the results.

## Building your own Cell-DEVS models
The folder `templates` contains a template project for asymmetric and classic Cell-DEVS models.
