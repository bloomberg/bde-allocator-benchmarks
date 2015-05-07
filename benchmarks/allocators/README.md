BDE Allocator Benchmarking Tools
================================
These are the programs used to produce the results found in
[N4468](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/n4468.pdf),
"On Quantifying Allocation Strategies".  Some of these programs have had bugs
fixed since the paper was produced, so may not produce identical results.

These programs depend upon:
  * Clang version 3.6 or later, or gcc version 5.1 or later, built with
   support for link-time-optimization (LTO, see FAQ)
  * For clang, libc++ version 3.6, or later
  * For gcc 5.1, libstdc++ patched according to:
   https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66055
  * [BDE Tools](https://github.com/bloomberg/bde-tools), which contains a
    custom build system based on [waf](https://github.com/waf-project/waf)
  * If building for LTO, /usr/bin/ld indicating ld.gold

A suitably-configured system can be built quickly using 
[Docker](https://docker.com); see the 'Docker' section below for instructions.

This repository contains a snapshot of the BDE library patched to make use
of various C++14 features required for the benchmark programs.  The patches
are included separately here in the [bde-patches-minimal] and 
[bde-patches-opt] files.

To configure,
```
  $ vi Makefile    # configure per instructions
```

To build,
```
  $ make build
```

To run benchmarks,
```
  $ make run
```

Other targets of interest:
```
  bde growth locality zation tention growth-orig
  run-growth run-locality run-zation run-tention
  clean
```

   progran    | section    | what
 -------------|------------|--------------------------------------------------
  growth.cc   | section 7  | Creating/destroying isolated basic data structures
  locality.cc | section 8  | Variation in Locality (long running)
  zation.cc   | section 9  | Variation in Utilization
  tention.cc  | section 10 | Variation in Contention

Other files:

  file                 | what
 ----------------------|---------------------------------
  readme-growth.txt    | instructions to produce CSV of tables in the paper
  test-growth          | scripts to run the benchmarks as published
  test-locality        |
  test-zation          |
  test-tention         |
  bde-patches-minimal  | snapshot of patches to bde that this depends on
  bde-patches-opt      | snapshot of an optimization for (multi-)pool

Docker
======
Two Dockerfiles have been provided which can be used to construct images with
all necessary tools and configuration for building these programs. One file 
builds a Debian 'testing' (Stretch) based image, and the other builds an Ubuntu
15.04 (Vivid) based image.

Debian Stretch
--------------
To use the Debian Stretch Dockerfile, follow these steps:
```
# Build an image called 'debian-stretch-clang'
docker build -f ./docker/debian-stretch-clang --rm -t debian-stretch-clang ./docker

# Build the BDE library and benchmarks using the image
docker run --rm -v $(git rev-parse --show-toplevel):/src -w /src/benchmarks/allocators debian-stretch-clang make build

# Run the benchmarks
docker run --rm -v $(git rev-parse --show-toplevel):/src -w /src/benchmarks/allocators debian-stretch-clang make run
```

Ubuntu Vivid
------------
To use the Ubuntu Vivid Dockerfile, follow these steps:
```
# Build an image called 'ubuntu-vivid-clang'
docker build -f ./docker/ubuntu-vivid-clang --rm -t ubuntu-vivid-clang ./docker

# Build the BDE library and benchmarks using the image
docker run --rm -v $(git rev-parse --show-toplevel):/src -w /src/benchmarks/allocators ubuntu-vivid-clang make build

# Run the benchmarks
docker run --rm -v $(git rev-parse --show-toplevel):/src -w /src/benchmarks/allocators ubuntu-vivid-clang make run
```

FAQ
===
Q1: What about tcmalloc?

A1: In all our tests, tcmalloc was substantially slower than the default
    new/delete on the target platform.

Q2: Why do these depend on recent clang++/libc++ or g++/libstdc++?

A2: The tests, particularly growth.cc and locality.cc, depend on features of
  C++14: specifically, the containers' comprehensive observance of allocator-
  traits requirements to direct their memory management.

Q3: How can I build these with a non-LTO-enabled toolchain?

A3: Comment out the ```LTO``` variable value in ```Makefile```.
    
