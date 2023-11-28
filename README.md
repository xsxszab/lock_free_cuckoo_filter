### Introduction

A lock-free cuckoo filter implementation.

### Requirements

* CMake >= 3.4.1
* OpenSSL (install via `sudo apt install libssl-dev`, for generating MD5 fingerprint)

### Usage
* install `build-essential`, `cmake` and `libssl-dev`.
* create dir ./build under project root directory and cd into it.
* run `cmake ..`.
* run `make` to build both the filter shared library and test cases.
* executable test cases are under ./build/bin/
