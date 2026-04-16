#pragma once
#include <cstdint>
#include <vector>
static constexpr int NUM_PROBES = 89;
enum class Architecture {
    Volta,
    Ampere,
    Hopper,
    Blackwell
};
enum class InputFormat {
    FP16,
    BF16
};
struct Probe16 {
    uint16_t a[16];
    uint16_t b[16];
    uint32_t c;
};
extern const Probe16 probes_fp16[NUM_PROBES];
extern const Probe16 probes_bf16[NUM_PROBES];
std::vector<uint32_t> generate_fingerprint(Architecture arch, InputFormat fmt);
void print_fingerprint(const std::vector<uint32_t>& fp);
std::vector<uint32_t> load_fingerprint(const char* path);
int compare_fingerprints(const std::vector<uint32_t>& a, const std::vector<uint32_t>& b);
