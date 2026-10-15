// Niobium: tags every polynomial op recorded inside a marked region with
// (phase, level, kind, j, i) so the compiler can re-order it.  A tag is metadata
// only; it never changes what is recorded.  Without OPENFHE_CPROBES the scope
// is empty.
#ifndef LBCRYPTO_CRYPTO_CKKSRNS_NBTAG_H
#define LBCRYPTO_CRYPTO_CKKSRNS_NBTAG_H

#include <atomic>
#include <cstdint>

#ifdef OPENFHE_CPROBES
    #include "cprobes.h"
#endif

// External linkage on purpose: every translation unit shares one counter.
namespace lbcrypto {
// One EvalMod ciphertext multiplication (tensor + relinearisation): its own
// region, numbered in program order.
inline uint32_t NbNextRelinRegion() {
    static std::atomic<uint32_t> next{1};
    return next.fetch_add(1);
}
}  // namespace lbcrypto

namespace {
enum : uint32_t {
    NB_TAG_PHASE_CTS = 1, NB_TAG_PHASE_STC = 2, NB_TAG_PHASE_EVALMOD = 3,
    NB_TAG_PRE = 1, NB_TAG_ROT = 2, NB_TAG_MAC = 3, NB_TAG_HORN = 4,
    NB_TAG_HADD = 5, NB_TAG_DELTA = 6, NB_TAG_DOWN = 7, NB_TAG_GKEY = 8,
    NB_TAG_RELIN = 9
};

#ifdef OPENFHE_CPROBES
class NbTagScope {
public:
    NbTagScope() = default;
    NbTagScope(uint32_t ph, uint32_t lv, uint32_t kind, uint32_t j, uint32_t i) { set(ph, lv, kind, j, i); }
    NbTagScope(const NbTagScope&) = delete;
    NbTagScope& operator=(const NbTagScope&) = delete;
    ~NbTagScope() { set(0, 0, 0, 0, 0); }
    void set(uint32_t ph, uint32_t lv, uint32_t kind, uint32_t j, uint32_t i) {
        if (openfhe_cprobe_tag != nullptr)
            openfhe_cprobe_tag(ph, lv, kind, j, i);
    }
};
#else
struct NbTagScope {
    NbTagScope() = default;
    NbTagScope(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) {}
    void set(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) {}
};
#endif

class NbRelinScope : public NbTagScope {
public:
    NbRelinScope() : NbTagScope(NB_TAG_PHASE_EVALMOD, lbcrypto::NbNextRelinRegion(), NB_TAG_RELIN, 0, 0) {}
};
}  // namespace

#endif
