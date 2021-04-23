# The Cloud Simulator

## Build

From build directory do `cmake .. && make`.

## Usage

### MVP version

1) Specify configuration of the cloud ([sample](/config))
2) The binary is located in `src/simulator` folder. Run simulator
   with `--config path/to/config/directory` options
3) Log is printed to stdout and duplicated to `log.csv` in the directory with
   binary
4) Temporal CLI is printed to stderr at the start. 
   * By default the whole Cloud has name `cloud`, so it can be booted by `boot cloud` CLI command
   * Data centers are named as in the `cloud.yaml` spec
   * Servers are named as `SERVER_NAME-SERVER_SERIAL`, where `SERVER_NAME` is the `name` value specified in `spec.yaml`

## Dependencies

* [yaml-cpp](https://github.com/jbeder/yaml-cpp)
* [argparse](https://github.com/p-ranav/argparse)
* [spdlog](https://github.com/gabime/spdlog)

Dependencies are attached to the project using git submodules, to do not forget
to load them before compiling.

## Architecture

The simulator has a layered design, as most of the analogues do. Everything is
based on a discrete-time event loop, which gives ability of creating and
receiving events using special API.

On the top of the event loop there are representations of the infrastructure (
physical entities and virtual machines), management classes and API of the
simulator.

### TODO: the scheme

## Code structure

The code is divided on 5 catalogues, located at `src` folder:

* `util`: some helpers not related to main purpose of the program and
  SimulationLogger
* `events`: discrete event system with `EventLoop` and interfaces `Event`
  and `IActor`
* `infrastructure`: the interface `IResource` and derived from it `Server`
  , `DataCenter`, `Cloud`. Some classes for management as `ResourceRegister`
  and `VMStorage`. They break abstractions a bit and should be refactored.
* `core`: management and API classes such as `Manager`, `SimulatorConfig`
  and `IScheduler` - an interface for used-implemented schedulers
* `simulator/simulator.cpp` file with `main()`

### TODO: add dependency scheme

## Work plan for the nearest future

Look to the `Github Projects` tab.

## Used links

1) [Writing a Discrete Event Simulation: ten easy lessons](https://users.cs.northwestern.edu/~agupta/_projects/networking/QueueSimulation/mm1.html)

2) [DISSECT-CF: a simulator to foster energy-aware scheduling in infrastructure clouds](https://arxiv.org/pdf/1604.06581.pdf):
   strongly influenced by this paper
