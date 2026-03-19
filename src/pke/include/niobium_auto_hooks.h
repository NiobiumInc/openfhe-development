// Copyright (C) 2023-2026, All rights reserved by Niobium Microsystems.
// Niobium Auto-Facade hooks — transparent record/replay for unmodified OpenFHE programs.
//
// Declare g_replay_mode flag and niobium_auto::* hook functions called from:
//   - ciphertext-ser.h  (DeserializeFromFile / SerializeToFile specialisations)
//   - cryptocontext-ser.h (DeserializeFromFile specialisation for CryptoContext)
//   - cryptocontext.h   (lazy_init + early-return in EvalAdd/Mult/Rotate/Sub/Negate)
//
// Implementations live in src/AutoFacade.cpp in the niobium library.
//
// Only active when NIOBIUM_AUTO_FACADE is defined.

#pragma once

#ifdef NIOBIUM_AUTO_FACADE

#include <atomic>
#include <cstdint>
#include <memory>
#include <string>

// Pull in DCRTPoly (core header, always safe to include from PKE)
#include "lattice/lat-hal.h"

// Forward-declare PKE types (avoid heavy includes in this header)
namespace lbcrypto {

template <typename Element>
class CryptoContextImpl;

template <typename Element>
using CryptoContext = std::shared_ptr<CryptoContextImpl<Element>>;

template <typename Element>
class CiphertextImpl;

template <typename Element>
using Ciphertext = std::shared_ptr<CiphertextImpl<Element>>;

}  // namespace lbcrypto

// ---------------------------------------------------------------------------
// Global replay-mode flag.
// Defined in AutoFacade.cpp; read by header-level hooks in cryptocontext.h.
// When true, every EvalAdd/EvalMult/etc returns a cheap empty ciphertext
// immediately — the real work was already dispatched to hardware by replay().
// ---------------------------------------------------------------------------
extern bool g_replay_mode;
extern bool g_replay_hollow_recording;

// Counter for no-oped FHE operations during replay.
// Incremented by NiobiumAutoScheme::dummy(); read by tests to confirm
// that OpenFHE polynomial work was actually skipped.
extern std::atomic<uint64_t> g_replay_noop_count;

// ---------------------------------------------------------------------------
// Hook functions — implemented in AutoFacade.cpp
// ---------------------------------------------------------------------------
namespace niobium_auto {

// Called from DeserializeFromFile<CryptoContext<DCRTPoly>>.
// Swaps in NiobiumAutoScheme, stores the crypto context, and fires lazy_init.
// Takes cc by non-const reference so the scheme can be replaced in-place.
void on_deserialize_crypto_context(lbcrypto::CryptoContext<lbcrypto::DCRTPoly>& cc);

// Called from DeserializeFromFile<Ciphertext<DCRTPoly>>.
// Queues a tag_input call (to be flushed when lazy_init fires).
void on_deserialize_ciphertext(const std::string& filepath,
                               lbcrypto::Ciphertext<lbcrypto::DCRTPoly>& ct);

// Called from the first EvalAdd/EvalMult/EvalRotate/EvalSub/EvalNegate.
// Idempotent — only runs once.  Loads config, sets up the compiler session,
// flushes pending tag_input calls, then either starts recording or replays.
void lazy_init(const lbcrypto::CryptoContext<lbcrypto::DCRTPoly>& cc);

// Called from SerializeToFile<Ciphertext<DCRTPoly>>.
//   Recording mode: calls compiler().probe(stem, ct) and returns false
//                   (caller does the normal file write).
//   Replay mode:    calls compiler().result(cc, stem, hw_ct), writes hw_ct to
//                   filepath, and returns true (caller skips normal file write).
bool on_serialize_ciphertext(const std::string& filepath,
                              const lbcrypto::Ciphertext<lbcrypto::DCRTPoly>& ct);

// True while recording is in progress (between start() and stop()).
bool is_recording();

// Called from Decrypt before the actual decryption.
//   Recording: probes the ciphertext as an output
//   Replay: substitutes ct with the HW-computed result
// Returns false always (caller proceeds with normal Decrypt).
bool on_decrypt(lbcrypto::Ciphertext<lbcrypto::DCRTPoly>& ct);

// Calls lazy_init() then returns g_replay_mode.  Use this instead of checking
// g_replay_mode directly in compute methods so that the compiler is guaranteed
// to be initialised before the first FHE operation executes.
bool is_replaying();

// No-arg overload — uses the CryptoContext captured by on_deserialize_crypto_context.
void lazy_init();

// Returns the real (unwrapped) scheme if the given scheme is a
// NiobiumAutoScheme proxy; otherwise returns the input unchanged.
// Used by CryptoContextImpl::save to avoid serializing the proxy type.
std::shared_ptr<lbcrypto::SchemeBase<lbcrypto::DCRTPoly>> unwrap_scheme(
    const std::shared_ptr<lbcrypto::SchemeBase<lbcrypto::DCRTPoly>>& scheme);

}  // namespace niobium_auto

#endif  // NIOBIUM_AUTO_FACADE
