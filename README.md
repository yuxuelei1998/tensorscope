# Tensorscope: DP16A Automation Probe System

Tensorscope is an automated tool designed to dissect and characterize the numeric behaviors of NVIDIA Tensor Cores through microbenchmarking. By leveraging Discriminant Numeric Probes (DNPs), Tensorscope extracts unique "numeric fingerprints" that reveal microarchitectural details such as rounding modes, internal precision, accumulation strategies, and normalization behaviors.

## Features

- **Automated Probing**: Automatically detects GPU architecture and compiles appropriate CUDA kernels.
- **Multi-Precision Support**: Supports both **FP16** (Half Precision) and **BF16** (Bfloat16) data types.
- **Numeric Fingerprinting**: Generates fingerprints to identify:
  - Rounding modes (e.g., RNE, RZ, TC-Truncation).
  - Internal accumulator precision.
  - Subnormal support and zero handling.
  - Accumulation order and dot product width.
- **Visual Data Path Analysis**: Automatically generates ASCII art diagrams of the internal accumulation data path (e.g., Volta 4-step, Ampere 2-step, Hopper & Blackwell 1-step) directly in the report.
- **Hardware Support**: Compatible with a wide range of NVIDIA architectures including Volta, Turing, Ampere, Ada Lovelace, Hopper, and Blackwell.

## Project Structure

The project is organized by precision format:

```text
tensorscope/
├── tensorscope.py            # Main automation script
├── bf16/                     # Bfloat16 probing module
│   ├── src/                  # Source code (CUDA + C++)
│   ├── bin/                  # Compiled binaries
│   └── numeric_fingerprints/ # Generated fingerprint data
└── fp16/                     # Float16 probing module
    ├── src/                  # Source code (CUDA + C++)
    ├── bin/                  # Compiled binaries
    └── numeric_fingerprints/ # Generated fingerprint data
```

## Prerequisites

Ensure you have the following installed on your system:

- **OS**: Windows or Linux
- **Python**: 3.x
- **CUDA Toolkit**: `nvcc` compiler must be in your PATH.
- **C++ Compiler**: `g++` (MinGW on Windows or GCC on Linux) must be in your PATH.
- **NVIDIA Driver**: Capable of running CUDA kernels for your specific GPU.

## Usage

1. **Run the automation script**:

    ```bash
    python tensorscope.py
    ```

2. **Follow the interactive prompts**:
    - The script will auto-detect your GPU. If dealing with multiple GPUs or detection fails, you can manually select the target.
    - Select the precision to probe: `fp16` or `bf16`.

3. **Process Overview**:
    - **Step 1**: The script compiles the CUDA kernel (`*dp16a_wmma.cu`) for the specific GPU architecture.
    - **Step 2**: It compiles the host-side analysis tool (`ProbeDesign.cpp`).
    - **Step 3 (Step A)**: Runs the CUDA binary to generate the raw fingerprint.
    - **Step 4 (Step B)**: Runs the C++ analysis tool to interpret the fingerprint and report the numeric behaviors.

## Output Format

The tool generates a compact, easy-to-read report in the console, including visualization of the inferred internal data path:

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

1. **Target Unit Activation**: Isolates the specific Tensor Core operation (e.g., `wmma` instructions).
2. **Discriminant Numeric Probe Design**: Constructs specific input matrices sensitive to microarchitectural parameters.
3. **Internal Validation**: Uses expected behaviors to validate the integrity of the probe.
4. **Differential Analysis**: Compares observed outputs against theoretical models (like IEEE 754) to determine the exact hardware behavior.

## Troubleshooting

- **`nvcc not found`**: Ensure the CUDA Toolkit is installed and added to your system's PATH.
- **`g++ not found`**: Install MinGW (Windows) or build-essential (Linux) and ensure it's in your PATH.
- **Architecture Errors**: If the auto-detected architecture flag (e.g., `sm_86`) is incorrect, you can manually override it when prompted.
