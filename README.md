# Investigating possible OpenMP acceleration strategies for OpenFOAM

This repository investigates the viability of implementing OpenMP acceleration, either with multi-threading on the CPU, or with GPU offloading support in OpenMP 4. This is a good idea only if:
- It doesn't require significant refactoring to the code base
- It results in a significant speedup. In particular, GPU acceleration should yield a speedup of around 20x.

Other factors that may affect the decision to switch:
- Planning to switch to C++ views in the future for OpenFOAM operations?
- Comparing to OpenACC acceleration, or `stdpar` (C++17) from NVIDIA?

Typically, NVIDIA GPU profiling should be done with `nvprof` but It doesn't support all devices. Also, Github CI machine do not provide a GPU so benchmarking results that are supposed to correspond to GPUs actually run on CPUs.

Benchmarks are performed using [foamUT](https://github.com/FoamScience/foamUT) with Catch2 as a back-end. 
