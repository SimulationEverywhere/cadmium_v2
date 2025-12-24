# Cadmium 2

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

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

## License

Cadmium 2 is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

Copyright (c) 2021-present Román Cárdenas Rodríguez, Jon Menard, Guillermo Trabes, and jsoulier

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
