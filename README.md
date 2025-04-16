# NVRAM-Optimized Relational Database

A high-performance relational database system leveraging the unique properties of Non-Volatile Random Access Memory (NVRAM) to achieve significant performance improvements over traditional in-memory databases.

## Overview

This project implements a hybrid database architecture that stores indexes in volatile RAM and data in persistent NVRAM, combining the performance of in-memory databases with the durability of persistent storage. Through careful optimization of data structures and algorithms for byte-addressability and persistence, the system demonstrates substantial performance gains in standard benchmarks.

## Key Features

- **Hybrid Memory Architecture**: Maintains indexes in volatile RAM for speed while storing data persistently in NVRAM
- **Custom Free Space Manager**: Optimized for byte-addressable memory with efficient block merging and first-fit allocation
- **B+ Tree Indexing**: Modified for direct NVRAM pointers to eliminate serialization overhead
- **Innovative Write-Ahead Logging**: Stores pointers rather than data blocks, uniquely possible with byte-addressability
- **Multi-granular Lock Manager**: Supports both table and row-level locking with low contention
- **Client-Server Interface**: TCP/IP based communication with support for transactions and standard database operations

## Performance Results

Our database significantly outperforms Redis across all YCSB workloads:

| Workload | NVRAM-DB (ops/sec) | Redis (ops/sec) | Improvement |
|----------|-------------------:|----------------:|------------:|
| Load Phase | 11,494 | 9,009 | 1.28× |
| Workload A (50% read/50% update) | 29,412 | 12,658 | 2.32× |
| Workload B (95% read/5% update) | 32,258 | 11,628 | 2.77× |
| Workload C (100% read) | 35,714 | 11,111 | 3.21× |
| Workload D (95% read/5% insert) | 31,250 | 11,905 | 2.62× |

### Latency Comparison (microseconds)

| Operation | NVRAM-DB | Redis | Improvement |
|-----------|--------:|------:|------------:|
| Read Operations | 46.99 | 168.79 | 3.6× |
| Update Operations | 79.93 | 110.89 | 1.4× |
| Insert Operations | 218.67 | 428.09 | 2.0× |
| Cleanup Overhead | 1.5-4.0 | 234-720 | 58-180× |

## Technical Architecture

The system consists of five main components:

1. **Memory Management Subsystem**: Efficiently allocates and deallocates space within the NVRAM region
2. **Indexing Structure**: Implements a B+ tree for fast data access while keeping only data in NVRAM
3. **Write-Ahead Logging (WAL)**: Ensures transaction durability and atomicity using NVRAM-optimized techniques
4. **Lock Manager**: Provides concurrency control for multi-threaded access with minimal contention
5. **Database Interface**: Exposes operations to applications via a client-server architecture

## Implementation Highlights

- **NVRAM Simulation**: Uses Device DAX namespace configuration to simulate NVRAM characteristics on conventional hardware
- **Persistence Instructions**: Leverages Intel CPU intrinsics for optimal cache line flushing and atomic updates
- **Transaction Management**: Implements ACID-compliant transactions with optimistic concurrency control
- **Recovery Mechanism**: Provides crash recovery through WAL replay
- **Client-Server Protocol**: Simple text-based protocol for database operations

## Building and Running

### Prerequisites

- Linux-based OS (tested on Debian 12)
- GCC compiler with C11 support
- At least 16GB of system RAM (2GB reserved for NVRAM simulation)

### Setting up NVRAM Simulation

1. Configure GRUB to reserve RAM:
   ```
   sudo nano /etc/default/grub
   # Add to GRUB_CMDLINE_LINUX_DEFAULT
   GRUB_CMDLINE_LINUX_DEFAULT="quiet memmap=2G!12G"
   sudo update-grub
   sudo reboot
   ```

2. Create a DAX namespace:
   ```
   sudo apt-get install ndctl
   sudo ndctl create-namespace --mode=devdax --size=2G --region=region0
   ```

### Compilation and Execution

1. Compile the server and client:
   ```
   cd src_db_v3_2
   make
   ```

2. Run the server:
   ```
   sudo make server
   ```

3. In another terminal, run the client:
   ```
   make client
   ```

## Future Work

- Secondary index implementation
- Distributed architecture for horizontal scaling
- Hybrid storage tiering for larger datasets
- SQL query interface
- Advanced recovery mechanisms
- Cache-line optimizations for NVRAM

## Documentation

- [Project Report](https://drive.google.com/file/d/17bDsvFcicoeH_0PdnUEExI1pG0lu8iT5/view)
- [Project Poster](https://drive.google.com/file/d/1qHQBTXKzCqPqGM3uWURBXLA2Rivw29Q8/view)

## Acknowledgments

This project was developed under the guidance of Dr. Gautum Barua at the Indian Institute of Information Technology Guwahati.

## License

[MIT License](LICENSE)