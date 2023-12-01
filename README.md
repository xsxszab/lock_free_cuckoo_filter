### Introduction

A lock-free cuckoo filter implementation.

### Requirements
* C++ 14
* CMake >= 3.4.1
* OpenSSL (install via `sudo apt install libssl-dev`, for generating MD5 fingerprint)

### Usage
* install `build-essential`, `cmake` and `libssl-dev`.
* create dir ./build under project root directory and cd into it.
* run `cmake ..`.
* run `make` to build both the filter shared library and test cases. New test cases can be added by creating .cpp file(s) under ./test, the CMake script will automatically detect and build new test cases.
* executable test cases are under ./build/bin/

### References
[Cuckoo Filter: Practically Better Than Bloom](https://www.cs.cmu.edu/~dga/papers/cuckoo-conext2014.pdf)

[Lock-free Cuckoo Hashing](https://ieeexplore.ieee.org/document/6888938)

[MemC3: Compact and Concurrent MemCache with Dumber Caching and Smarter Hashing](https://www.usenix.org/system/files/conference/nsdi13/nsdi13-final197.pdf)

[Hazard pointers: safe memory reclamation for lock-free objects](https://ieeexplore.ieee.org/document/1291819)
