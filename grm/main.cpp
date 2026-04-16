#include "grm.h"
#include <cstdio>
#include <cstring>
static void usage(const char* prog)
{
    std::fprintf(stderr,
        "Usage: %s <arch> <fmt> [reference_file]\n"
        "  arch : volta | ampere | hopper | blackwell\n"
        "  fmt  : fp16  | bf16\n"
        "  Volta+BF16 is not supported.\n",
        prog);
}
static const char* arch_name(Architecture a)
{
    switch (a) {
        case Architecture::Volta:     return "Volta";
        case Architecture::Ampere:    return "Ampere";
        case Architecture::Hopper:    return "Hopper";
        case Architecture::Blackwell: return "Blackwell";
    }
    return "?";
}
int main(int argc, char* argv[])
{
    if (argc < 3) { usage(argv[0]); return 1; }
    Architecture arch;
    if      (std::strcmp(argv[1], "volta")     == 0) arch = Architecture::Volta;
    else if (std::strcmp(argv[1], "ampere")    == 0) arch = Architecture::Ampere;
    else if (std::strcmp(argv[1], "hopper")    == 0) arch = Architecture::Hopper;
    else if (std::strcmp(argv[1], "blackwell") == 0) arch = Architecture::Blackwell;
    else {
        std::fprintf(stderr, "Unknown arch '%s'\n", argv[1]);
        usage(argv[0]);
        return 1;
    }
    InputFormat fmt;
    if      (std::strcmp(argv[2], "fp16") == 0) fmt = InputFormat::FP16;
    else if (std::strcmp(argv[2], "bf16") == 0) fmt = InputFormat::BF16;
    else {
        std::fprintf(stderr, "Unknown format '%s'\n", argv[2]);
        usage(argv[0]);
        return 1;
    }
    auto grm_fp = generate_fingerprint(arch, fmt);
    if (grm_fp.empty()) {
        std::fprintf(stderr, "Error: %s + BF16 is not supported.\n", arch_name(arch));
        return 1;
    }
    if (argc < 4) {
        std::printf("GRM fingerprint — %s / %s (%d probes)\n",
                    arch_name(arch),
                    (fmt == InputFormat::FP16) ? "FP16" : "BF16",
                    NUM_PROBES);
        print_fingerprint(grm_fp);
        return 0;
    }
    const char* ref_path = argv[3];
    auto ref_fp = load_fingerprint(ref_path);
    if (ref_fp.empty()) {
        std::fprintf(stderr, "Error: failed to load reference from '%s'\n", ref_path);
        return 1;
    }
    std::printf("Comparing GRM (%s/%s) against '%s'\n",
                arch_name(arch),
                (fmt == InputFormat::FP16) ? "FP16" : "BF16",
                ref_path);
    int mismatches = compare_fingerprints(grm_fp, ref_fp);
    if (mismatches == 0) {
        std::printf("PASS — all %d probes match.\n", NUM_PROBES);
    } else {
        std::printf("FAIL — %d/%d probes differ.\n", mismatches, NUM_PROBES);
    }
    return (mismatches == 0) ? 0 : 1;
}
