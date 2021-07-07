# cluon-lwe450

Copyright 2021 RISE Research Institute of Sweden - Maritime Operations. Licensed under the Apache License Version 2.0. For details, please contact Fredrik Olsson (fredrik.x.olsson(at)ri.se).

A [libcluon](https://github.com/chrberger/libcluon)-based microservice for listening in on a multicast-based LWE450 network. This software does not perform any parsing of the LWE450 messages, merely assembles any fragmented messages into full messages. It can be run in two modes:
* `gather`, join a multicast group part of a LWE450 network and start receiving message and either:
  * publish to an OD4 session, or
  * log directly to disk (`--standalone`)
* `log`, listen to an OD4 session for raw LWE450 messages from other `gatherers`  and dump these to an aggregated log file on disk

## How do I get it?
Each release of `cluon-lwe450` is published as a docker image [here](https://github.com/orgs/RISE-MO/packages/container/package/cluon-lwe450) and is publicly available.

Can also be used as a standalone commandline tool. No pre-built binaries are, however, provided for this purpose.

## Example docker-compose setup
The example below showcases a setup with two gatherers (listening on two separate stream (one UDP and one TCP)) and one logger that aggregates published messages from the gatherers into a single file.
```yaml
version: '2'
services:    
    gatherer_1:
        container_name: cluon-lwe450-gatherer-1
        image: ghcr.io/rise-mo/cluon-lwe450:v0.1.0
        restart: on-failure
        network_mode: "host"
        command: "--cid 111 --id 1 gather -a 239.192.0.2 -p 60002"
    gatherer_2:
        container_name: cluon-lwe450-gatherer-2
        image: ghcr.io/rise-mo/cluon-lwe450:v0.1.0
        restart: on-failure
        network_mode: "host"
        command: "--cid 111 --id 2 gather -a 239.192.0.3 -p 60003"
    logger:
        container_name: cluon-lwe450-logger
        image: ghcr.io/rise-mo/cluon-lwe450:v0.1.0
        restart: on-failure
        network_mode: "host"
        volumes:
        - .:/opt/cluon-lwe450
        command: "--cid 111 --path /opt/cluon-lwe450/recordings/lwe450.log log"
```

## Details

### Message set
This microservice introduce a new OD4-compatible message type (`raw.LWE450`) in the `risemo` message set. See `src/risemo-message-set.odvd` for details.

### Build from source
This repository makes use of [CPM.cmake](https://github.com/cpm-cmake/CPM.cmake) for dependency resolution as an interal part of the CMake setup. As a result, the only requirements for building from source are:
* a C++17 compliant compiler
* CMake (>=3.14)

As part of the CMake configuration step, the following dependencies are downloaded and configured:
* [libcluon](https://github.com/chrberger/libcluon)
* [CLI11](https://github.com/CLIUtils/CLI11)
* [spdlog](https://github.com/gabime/spdlog)
* [doctest](https://github.com/onqtam/doctest) (for testing only)

To build (from the root directory of this repo):
```cmd
cmake -Bbuild -DCMAKE_BUILD_TYPE=Release
cmake --build build -- -j 8
```

### Tests

Unit tests for the lwe450 message assembler is compiled into the executable `cluon-lwe450-tester`.

To run tests (after successful build):
```cmd
cd build
ctest -C Debug -T test --output-on-failure --
```


### Development setup
This repo contains some configuration files (in the `.vscode`-folder) for getting started easy on the following setup:
* Ubuntu 20.04 (WSL2 is fine)
* GCC 9
* python 3
* VSCode as IDE, using the following extensions:
  - C/C++ (ms-vscode.cpptools)
  - C/C++ Extension Pack (ms-vscode.cpptools-extension-pack)
  - CMake Tools (ms-vscode.cmake-tools)
  - Python (ms-python.python)

Do the following steps to get started:
* Clone repo
* Create a python virtual environment (`python3 -m venv venv`) in the root of the repo.
* Open vscode in the repo root (`code .`)

The provided configuration is very lightweight and should be easily adaptable to other enviroments/setups.

