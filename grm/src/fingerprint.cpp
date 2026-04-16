#include "grm.h"
#include "volta_tensor.h"
#include "ampere_tensor.h"
#include "hopper_tensor.h"
#include "blackwell_tensor.h"
#include <cstdio>
#include <cstdlib>
#include <cctype>
std::vector<uint32_t> generate_fingerprint(Architecture arch, InputFormat fmt)
{
    if (fmt == InputFormat::BF16 && arch == Architecture::Volta) {
        return {};
    }
    const Probe16* probes = (fmt == InputFormat::FP16) ? probes_fp16 : probes_bf16;
    std::vector<uint32_t> result;
    result.reserve(NUM_PROBES);
    for (int i = 0; i < NUM_PROBES; ++i) {
        const Probe16& p = probes[i];
        uint32_t out = 0;
        if (fmt == InputFormat::FP16) {
            const fp16_t* a = p.a;
            const fp16_t* b = p.b;
            fp32_t        c = p.c;
            switch (arch) {
                case Architecture::Volta:
                    out = volta_dp4a_fp32(a, b, 16, c);
                    break;
                case Architecture::Ampere:
                    out = ampere_dp8a_fp32(a, b, 16, c);
                    break;
                case Architecture::Hopper:
                    out = hopper_dp16a_fp32(a, b, 16, c);
                    break;
                case Architecture::Blackwell:
                    out = blackwell_dp16a_fp32(a, b, 16, c);
                    break;
            }
        } else {
            const bf16_t* a = p.a;
            const bf16_t* b = p.b;
            fp32_t        c = p.c;
            switch (arch) {
                case Architecture::Volta:
                    break;
                case Architecture::Ampere:
                    out = ampere_dp8a_bf16(a, b, 16, c);
                    break;
                case Architecture::Hopper:
                    out = hopper_dp16a_bf16(a, b, 16, c);
                    break;
                case Architecture::Blackwell:
                    out = blackwell_dp16a_bf16(a, b, 16, c);
                    break;
            }
        }
        result.push_back(out);
    }
    return result;
}
void print_fingerprint(const std::vector<uint32_t>& fp)
{
    for (int i = 0; i < (int)fp.size(); ++i) {
        std::printf("[%2d] 0x%08X\n", i, fp[i]);
    }
}
std::vector<uint32_t> load_fingerprint(const char* path)
{
    std::vector<uint32_t> result;
    FILE* f = std::fopen(path, "r");
    if (!f) {
        std::fprintf(stderr, "Error: cannot open '%s'\n", path);
        return result;
    }
    char line[256];
    while (std::fgets(line, sizeof(line), f)) {
        const char* p = line;
        while (*p && std::isspace((unsigned char)*p)) ++p;
        if (*p == '\0' || *p == '#') continue;
        char* end = nullptr;
        uint32_t val = (uint32_t)std::strtoul(p, &end, 16);
        if (end != p) {
            result.push_back(val);
        }
    }
    std::fclose(f);
    return result;
}
int compare_fingerprints(const std::vector<uint32_t>& a, const std::vector<uint32_t>& b)
{
    int mismatches = 0;
    int n = (int)std::min(a.size(), b.size());
    for (int i = 0; i < n; ++i) {
        if (a[i] != b[i]) {
            ++mismatches;
            std::printf("MISMATCH [%2d]: GRM=0x%08X  HW=0x%08X\n", i, a[i], b[i]);
        }
    }
    if (a.size() != b.size()) {
        std::printf("WARNING: size mismatch — GRM has %zu probes, file has %zu\n",
                    a.size(), b.size());
        ++mismatches;
    }
    return mismatches;
}
