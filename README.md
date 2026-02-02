# Dokumentation for GPI-Benchmarks

## Introduction

The following contains a documentation on how to use the Benchmarks for GPI-2. These Benchmarks can be used to test bandwidth and latency for different usecases.
GPI-2 is an API for asynchronous communication, which implements the GASPI specification. It provides a flexible, scalable and fault tolerant interface for parallel applications. (https://github.com/cc-hpc-itwm/GPI-2)

## Dependencies

* GPI-2
* MPI (optional)

## Installation
```
$ mkdir build && cd build
$ cmake -DCMAKE_INSTALL_PREFIX=<PATH> -DWITH_MPI=<yes|no> ..
$ make -j
$ make install
```

### How to run
```
$ mpirun -np 2 ./gbs_write_bw
$ gaspi_run -n 2 -m <machine_file> ./gbs_write_bw
```

### Available Parameters
Each benchmark provides a specialized set of available parameters. use ```-h, --help``` to list them:
```
$ ./gbs_write_bw --help
Usage: [options]
Options:
         -h [--help]    Display this help message.
         -w [--window-size] arg  Number of messages sent per iteration. Default 64.
         -s [--min-message-size] arg     Minimum message size. Default 1 byte.
         -e [--max-message-size] arg     Maximum message size. Default (1 << 22) byte.
         -b [--single-buffer]   Use a single memory allocation for the measurements.
         -v [--verify]  Check results of the performed operation.
         -i [--iterations] arg  Number of iterations. Default 10.
         -u [--warmup-iterations] arg   Number of warmup iterations. Default 10.
         --csv  Print output in csv format with statistics.
         --raw-csv      Print the collected raw data without statistics.
```

### Cite as
If you use **GPI-2 Benchmarks** in your research or publications, please cite the following paper:  
```bibtex
@INPROCEEDINGS{Bartelheimer_2025,
  author={Bartelheimer, Niklas J. and Neuwirth, Sarah M.},
  booktitle={2025 33rd International Symposium on Modeling, Analysis and Simulation of Computer and Telecommunication Systems (MASCOTS)}, 
  title={Comprehensive Performance Analysis of Portals4 Communication Primitives on BXI Hardware}, 
  year={2025},
  volume={},
  number={},
  pages={1-8},
  keywords={Analytical models;Computational modeling;Bandwidth;Benchmark testing;Libraries;Hardware;Performance analysis;Telecommunications;Electronics packaging;MPI;GASPI;PGAS;BXI;Portals4;Performance Study;Benchmarking;Performance Analysis},
  doi={10.1109/MASCOTS67699.2025.11283317}}
```
