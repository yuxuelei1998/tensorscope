# Tensorscope: Tensor Core Numeric Fingerprinting System

Tensorscope is an automated tool for dissecting and characterizing the numeric behaviors of NVIDIA Tensor Cores through microbenchmarking. By designing Discriminant Numeric Probes (DNPs), Tensorscope extracts unique "numeric fingerprints" that reveal microarchitectural details such as rounding modes, internal precision, accumulation order, and dot-product width.

## Features

- **Automated Probing**: Auto-detects GPU architecture and compiles appropriate CUDA kernels.
- **Multi-Precision Support**: FP16 (half-precision) and BF16 (bfloat16) input formats.
- **Numeric Fingerprinting**: 89-probe fingerprint identifies:
  - Rounding mode (RNE vs. TC-Truncation)
  - Internal accumulator precision (extra mantissa bits)
  - Dot-product group width (4 / 8 / 16)
  - Accumulation order and sequential group topology
  - Subnormal handling and signed-zero propagation
  - NaN / Inf pass-through behavior
- **Golden Reference Model (GCM)**: Software simulator that generates bit-exact expected fingerprints for each architecture, enabling differential analysis without a reference GPU.
- **Visual Data Path**: ASCII diagram of the inferred internal accumulation path (e.g., Volta 4-step, Ampere 2-step, Hopper/Blackwell 1-step).
- **Hardware Support**: Volta, Turing, Ampere, Ada Lovelace, Hopper, Blackwell.

## Project Structure

```text
tensorscope/
├── tensorscope.py              # Main automation script
├── fp16/                       # FP16 probing module
│   ├── src/                    # CUDA kernel + C++ analysis tool
│   ├── bin/                    # Compiled binaries
│   ├── fp16_dp16a/             # Probe input vectors
│   └── numeric_fingerprints/   # Reference hardware fingerprints
├── bf16/                       # BF16 probing module (same layout as fp16/)
│   ├── src/
│   ├── bin/
│   ├── bf16_dp16a/
│   └── numeric_fingerprints/
└── gcm/                        # Golden Reference Model
    ├── CMakeLists.txt
    ├── include/gcm.h
    ├── src/
    │   ├── probes_fp16.cpp     # 89 FP16 probe input definitions
    │   ├── probes_bf16.cpp     # 89 BF16 probe input definitions
    │   └── fingerprint.cpp     # generate / compare / print fingerprints
    └── main.cpp                # CLI: gcm <arch> <fmt> [reference_file]
```

## Prerequisites

| Requirement | Notes |
|---|---|
| OS | Windows or Linux |
| Python 3.x | For the automation script |
| CUDA Toolkit | `nvcc` must be in PATH |
| C++ compiler | g++ (MinGW on Windows / GCC on Linux) |
| CMake ≥ 3.14 | For building the GCM |
| NVIDIA driver | Matching your GPU |

## Quick Start

### 1. Run hardware probing

```bash
python tensorscope.py
```

Follow the prompts to select precision (`fp16` or `bf16`). The script will:

1. Compile the CUDA kernel (`*dp16a_wmma.cu`) for the detected GPU architecture.
2. Compile the host-side analysis tool (`ProbeDesign.cpp`).
3. Run the CUDA binary to capture the raw 89-value fingerprint.
4. Run the analysis tool to interpret the fingerprint and print the report.

### 2. Build and run the Golden Reference Model

The GCM depends on [tensor_sim](https://github.com/yuxuelei1998/tensor_sim), a software simulator for NVIDIA Tensor Core operations. Clone it as a sibling directory of this repository before building:

```bash
# Directory layout expected:
#   <parent>/
#   ├── tensorscope/   ← this repo
#   └── tensor_sim/    ← simulator dependency

git clone https://github.com/yuxuelei1998/tensor_sim ../tensor_sim
```

Then build the GCM:

```bash
cd gcm
mkdir build && cd build
cmake .. -G "MinGW Makefiles"   # or "Unix Makefiles" on Linux
make -j4
```

Generate a GCM fingerprint:

```bash
./gcm <arch> <fmt>
# e.g.
./gcm hopper fp16
./gcm ampere bf16
```

Compare GCM against a hardware-captured fingerprint:

```bash
./gcm ampere fp16 ../../fp16/numeric_fingerprints/"NVIDIA Ampere Tensor Core.txt"
./gcm hopper bf16 ../../bf16/numeric_fingerprints/"NVIDIA Hopper & Blackwell Tensor Core.txt"
```

A passing run prints:

```
PASS — all 89 probes match.
```

## GCM Architecture / Format Support

| Architecture | FP16 | BF16 | Dot-product width |
|---|---|---|---|
| Volta | ✓ | — | 4 |
| Ampere | ✓ | ✓ | 8 |
| Hopper | ✓ | ✓ | 16 |
| Blackwell | ✓ | ✓ | 16 |

## Probe Index Map (89 probes)

| Range | Category |
|---|---|
| [0] | Signed-zero propagation |
| [1–19] | NaN / Inf handling |
| [20–35] | A-operand subnormal sweep (positions 0–15) |
| [36–51] | B-operand subnormal sweep (positions 0–15) |
| [52] | Both operands subnormal |
| [53] | C accumulator subnormal pass-through |
| [54–55] | Rounding boundary probes (RNE vs. TC-Trunc) |
| [56–71] | Accumulation order / group topology (positions k=0–15) |
| [72–75] | RNE precision (extra accumulator bit probes) |
| [76–79] | TC-Trunc precision (extra accumulator bit probes) |
| [80–87] | Monotonicity / swamping threshold |
| [88] | TC-Trunc probe 3 (sum = 2 − 2⁻⁴⁰) |

## Sample Output

```text
===================================================================================================================
                                           NUMERIC PROBE ANALYSIS REPORT
===================================================================================================================
+--------------------------+---------------------------------------------------------------------------------------+
| PROBE TYPE               | RESULT FEEDBACK                                                                       |
+--------------------------+---------------------------------------------------------------------------------------+
| Signed Zero              | +0                                                                                    |
| NaN & INF                | Fixed NaN: 0x7fffffff                                                                 |
| ...                      | ...                                                                                   |
| Internal Data Path       | 2-Group Sequential (Width 8)                                                          |
|                          |    pd[00-07] pd[08-15]                                                                |
|                          |         |         |                                                                   |
|                          | C --+->(+)----+->(+)----> D                                                           |
+--------------------------+---------------------------------------------------------------------------------------+
| HARDWARE IDENTIFICATION  | Matches Hardware: NVIDIA Ampere Tensor Core                                           |
+--------------------------+---------------------------------------------------------------------------------------+
```

## Methodology

Tensorscope operates in four phases:

1. **Target Unit Activation** — Isolate the specific Tensor Core operation via WMMA instructions (`wmma::mma_sync`).
2. **Discriminant Numeric Probe Design** — Construct input vectors sensitive to a single microarchitectural parameter while keeping all others neutral.
3. **Internal Validation** — Cross-check hardware outputs against the GCM (software simulator) to confirm probe integrity.
4. **Differential Analysis** — Compare the 89-value fingerprint against the reference database to identify the exact hardware behavior.

## Troubleshooting

- **`nvcc not found`**: Install the CUDA Toolkit and add its `bin/` directory to PATH.
- **`g++ not found`**: Install MinGW (Windows) or `build-essential` (Linux).
- **Architecture mismatch**: If auto-detection picks the wrong SM version, override it when prompted.
- **GCM build fails**: Ensure `tensor_sim` is cloned as a sibling of this repository (see above), and that CMake ≥ 3.14 and g++ are in PATH.
