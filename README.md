
# ZergMap

ZergMap is a C program designed to decode a custom command and control (C2) protocol inside PCAP files. It can also ingest text files to encode the C2 protocol and create a PCAP. The program includes custom packet parsing and utilizes various data structures and algorithms to manage maps, find paths, and optimize performance.

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [File Descriptions](#file-descriptions)
- [Installation](#installation)
- [Usage](#usage)
- [Testing](#testing)
- [Contributing](#contributing)
- [License](#license)

## Overview

ZergMap is built to handle complex map data, perform pathfinding, and handle custom packet parsing for C2 protocols. It is useful for applications requiring efficient spatial querying, route optimization, and custom network protocol handling.

## Features

- Decodes custom C2 protocols from PCAP files
- Encodes C2 protocols into text files and creates PCAPs
- Custom packet parsing
- Graph-based map representation
- Pathfinding algorithms
- Priority queue implementation
- Spatial partitioning with quadtrees
- Set operations for efficient data handling

## File Descriptions

### Core Files

- `zergmap.c`: Main program file for ZergMap.
- `zergmap.h`: Header file for the main program.
- `zergmap_func.h`: Header file containing function declarations for ZergMap.
- `zergmap-driver.c`: Driver program for testing and demonstrating ZergMap functionality.

### Data Structures

- `graph.c`: Implementation of graph data structure.
- `graph.h`: Header file for graph data structure.
- `map.c`: Implementation of map-related functions.
- `map.h`: Header file for map-related functions.
- `quadTree.c`: Implementation of quadtree data structure.
- `quadTree.h`: Header file for quadtree data structure.
- `priority-queue.c`: Implementation of priority queue.
- `priority-queue.h`: Header file for priority queue.
- `set.c`: Implementation of set operations.
- `set.h`: Header file for set operations.

### Pathfinding

- `pathfinding.c`: Implementation of pathfinding algorithms.
- `pathfinding.h`: Header file for pathfinding algorithms.

### Build Configuration

- `Makefile`: Build configuration for compiling the program.

### Test Scripts

- `test-case-1-easy_3n0r.sh`: Test script for an easy test case with specific conditions.
- `test-case-2-easy_3n1r.sh`: Test script for another easy test case.
- `test-case-3-easy_5nxx.sh`: Test script for an easy test case with different conditions.
- `test-case-4-easy_v6_2n0r.sh`: Test script for an easy test case with varied conditions.
- `test-case-5-med_4n1r.sh`: Test script for a medium difficulty test case.
- `test-case-6-hard_6n0r.sh`: Test script for a hard test case.
- `test-case-7-all-pcaps.sh`: Test script for running all PCAP test cases.
- `test-graph.c`: Test cases for graph functionality.
- `test-map.c`: Test cases for map functionality.
- `test-quadTree.c`: Test cases for quadtree functionality.
- `test-run.c`: Test cases for general program functionality.
- `test-valgrind-zergmap.sh`: Script for testing memory leaks and errors using Valgrind.
- `test-zergmap.c`: Additional test cases for ZergMap functionality.

## Installation

To compile and run ZergMap, you need to have a C compiler and `make` installed on your system. Follow these steps:

1. Clone the repository:
    ```sh
    git clone https://github.com/yourusername/zergmap.git
    cd zergmap
    ```

2. Build the program using `make`:
    ```sh
    make
    ```

## Usage

After building the program, you can run it using:
```sh
./zergmap
```

The driver program can be used for testing and demonstrating the features of ZergMap:
```sh
./zergmap-driver
```

## Testing

You can run the provided test scripts to validate the functionality of ZergMap. Each test script is designed to test different aspects of the program.

1. Navigate to the directory containing the test scripts:
    ```sh
    cd test
    ```

2. Run a specific test script:
    ```sh
    ./test-case-1-easy_3n0r.sh
    ```

Replace `test-case-1-easy_3n0r.sh` with the name of the test script you want to execute.



