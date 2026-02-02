# Dokumentation for GPI-Benchmarks

## Introduction

The following contains a documentation on how to use the Benchmarks for GPI-2. These Benchmarks can be used to test bandwidth and latency for different usecases.

GPI-2 is an API for asynchronous communication, which implements the GASPI specification. It provides a flexible, scalable and fault tolerant interface for parallel applications. (https://github.com/cc-hpc-itwm/GPI-2)



## Installation
Hier beschreiben wie man dies Installiert?

## Usage 

### 
Possible arguments are :
GASPI Micro Benchmark:
For bandwidth (bw):

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

For latency (lat):

         -h [--help]    Display this help message.
         -w [--window-size] arg  Number of messages sent per iteration. Default 64.
         -s [--min-message-size] arg     Minimum message size. Default 1 byte.
         -e [--max-message-size] arg     Maximum message size. Default (1 << 22) byte.
         -v [--verify]  Check results of the performed operation.
         -i [--iterations] arg  Number of iterations. Default 10.
         -u [--warmup-iterations] arg   Number of warmup iterations. Default 10.
         --csv  Print output in csv format with statistics.
         --raw-csv      Print the collected raw data without statistics.