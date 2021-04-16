# The Cloud Simulator

## Build

From build directory do `cmake .. && make`.

## Usage

To be added.

## Dependencies

* [yaml-cpp](https://github.com/jbeder/yaml-cpp)
* [argparse](https://github.com/p-ranav/argparse)
* [loguru](https://github.com/emilk/loguru)

Dependencies are attached to the project using git submodules, to do not forget
to load them before compiling.

## Architecture

The simulator has a layered design, as most of the analogues do. Everything is
based on a discrete-time event loop, which gives ability of creating and
receiving events using special API.

On the top of the event loop there are representations of physical entities and
virtual machines, management classes and API of the simulator.

### TODO: the scheme

## Code structure

The code is divided on 5 catalogues, located at `src` folder:

* `util`
* `events`
* `infrastructure`
* `core`
* `simulator/simulator.cpp` file with `main()`

### TODO: add brief descriptions of modules + dependency scheme

## Used links

1) [Writing a Discrete Event Simulation: ten easy lessons](https://users.cs.northwestern.edu/~agupta/_projects/networking/QueueSimulation/mm1.html)

2) [DISSECT-CF: a simulator to foster energy-aware scheduling in infrastructure clouds](https://arxiv.org/pdf/1604.06581.pdf):
   strongly influenced by this paper
