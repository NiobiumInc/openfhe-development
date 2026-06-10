// Copyright (C) 2023-2026, All rights reserved by Niobium Microsystems.
// Niobium Auto-Facade hooks — weak default for niobium_auto::on_make_plaintext.
//
// Patched cryptocontext.h (MakePackedPlaintext / MakeCKKSPackedPlaintext) calls
// niobium_auto::on_make_plaintext after encoding. The symbol is normally
// provided by a host auto-facade — libnbfhetch's weak stub, or the strong
// override in niobium-client / niobium-compiler's AutoFacade. Consumers that
// link the instrumented OpenFHE directly without any of those (e.g.
// niobium-compiler's nbformal, which links libOPENFHEpke but not a facade)
// would otherwise fail to link: libOPENFHEpke itself references the symbol, and
// on ELF the executable link errors on the unresolved reference (macOS only
// defers it). Ship a weak no-op default here so libOPENFHEpke is self-contained;
// a strong override still wins at link time.
//
// Only active when OPENFHE_CPROBES is defined (matching niobium_auto_hooks.h).

// openfhe.h pulls in the lbcrypto types (Plaintext, Ciphertext, CryptoContext,
// SchemeBase) that niobium_auto_hooks.h's declarations reference. This mirrors
// the include set in niobium-compiler's AutoFacade.cpp, which defines the same
// hooks.
#include "openfhe.h"
#include "niobium_auto_hooks.h"

#ifdef OPENFHE_CPROBES

#if defined(__GNUC__) || defined(__clang__)
  #define NB_WEAK __attribute__((weak))
#else
  #define NB_WEAK
#endif

namespace niobium_auto {

NB_WEAK void on_make_plaintext(lbcrypto::Plaintext& /*pt*/) {
    // Default no-op. A host auto-facade provides a strong override that
    // tags the plaintext as a live-in input for record/replay.
}

}  // namespace niobium_auto

#endif  // OPENFHE_CPROBES
