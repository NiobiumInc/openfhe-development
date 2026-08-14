// Copyright (C) 2023-2026, All rights reserved by Niobium Microsystems.
// NiobiumAutoScheme — replay-mode proxy wrapping SchemeBase<DCRTPoly>.
// NiobiumAutoFHE    — replay-mode proxy wrapping FHEBase<DCRTPoly>.
//
// Swapped in by AutoFacade.cpp after CryptoContext deserialization.
// In replay mode (g_replay_mode == true), all compute operations return
// empty dummy ciphertexts immediately with zero polynomial work.
// All other calls (key generation, serialization, etc.) forward to the
// wrapped real scheme.
//

#pragma once

#include "schemebase/base-scheme.h"
#include "niobium_auto_hooks.h"
#include "Utils/ScopedPause.h"

namespace lbcrypto {

// ---------------------------------------------------------------------------
// NiobiumAutoFHE — proxy for FHEBase<DCRTPoly> covering bootstrapping ops
// ---------------------------------------------------------------------------

class NiobiumAutoFHE final : public FHEBase<DCRTPoly> {
    std::shared_ptr<FHEBase<DCRTPoly>> m_real;
    std::weak_ptr<CryptoContextImpl<DCRTPoly>> m_cc;

    static Ciphertext<DCRTPoly> dummy(ConstCiphertext<DCRTPoly>& ct) {
        return std::make_shared<CiphertextImpl<DCRTPoly>>(
            ct->GetCryptoContext(), ct->GetKeyTag(), ct->GetEncodingType());
    }
    Ciphertext<DCRTPoly> dummy_from_cc() const {
        auto cc = m_cc.lock();
        return cc ? std::make_shared<CiphertextImpl<DCRTPoly>>(cc) : nullptr;
    }

public:
    NiobiumAutoFHE(std::shared_ptr<FHEBase<DCRTPoly>> real,
                   std::weak_ptr<CryptoContextImpl<DCRTPoly>> cc)
        : m_real(std::move(real)), m_cc(std::move(cc)) {}

    // Setup / key-gen: always forward (with pause to exclude from trace)
    void EvalBootstrapSetup(const CryptoContextImpl<DCRTPoly>& cc,
                            std::vector<uint32_t> levelBudget,
                            std::vector<uint32_t> dim1, uint32_t slots,
                            uint32_t correctionFactor, bool precompute,
                            bool BTSlotsEncoding) override {
        niobium::ScopedPause pause;
        m_real->EvalBootstrapSetup(cc, levelBudget, dim1, slots, correctionFactor,
                                   precompute, BTSlotsEncoding);
    }
    std::shared_ptr<std::map<uint32_t, EvalKey<DCRTPoly>>> EvalBootstrapKeyGen(
        const PrivateKey<DCRTPoly> sk, uint32_t slots) override {
        niobium::ScopedPause pause;
        return m_real->EvalBootstrapKeyGen(sk, slots);
    }
    void EvalBootstrapPrecompute(const CryptoContextImpl<DCRTPoly>& cc,
                                 uint32_t slots) override {
        niobium::ScopedPause pause;
        m_real->EvalBootstrapPrecompute(cc, slots);
    }
    void EvalFBTSetup(const CryptoContextImpl<DCRTPoly>& cc,
                      const std::vector<std::complex<double>>& coeffs,
                      uint32_t numSlots, const BigInteger& PIn,
                      const BigInteger& POut, const BigInteger& Bigq,
                      const PublicKey<DCRTPoly>& pubKey,
                      const std::vector<uint32_t>& dim1,
                      const std::vector<uint32_t>& levelBudget,
                      uint32_t lvlsAfterBoot, uint32_t depthLeveledComputation,
                      size_t order) override {
        niobium::ScopedPause pause;
        m_real->EvalFBTSetup(cc, coeffs, numSlots, PIn, POut, Bigq, pubKey, dim1,
                             levelBudget, lvlsAfterBoot, depthLeveledComputation,
                             order);
    }
    void EvalFBTSetup(const CryptoContextImpl<DCRTPoly>& cc,
                      const std::vector<int64_t>& coeffs,
                      uint32_t numSlots, const BigInteger& PIn,
                      const BigInteger& POut, const BigInteger& Bigq,
                      const PublicKey<DCRTPoly>& pubKey,
                      const std::vector<uint32_t>& dim1,
                      const std::vector<uint32_t>& levelBudget,
                      uint32_t lvlsAfterBoot, uint32_t depthLeveledComputation,
                      size_t order) override {
        niobium::ScopedPause pause;
        m_real->EvalFBTSetup(cc, coeffs, numSlots, PIn, POut, Bigq, pubKey, dim1,
                             levelBudget, lvlsAfterBoot, depthLeveledComputation,
                             order);
    }

    // Compute: short-circuit in replay mode
    Ciphertext<DCRTPoly> EvalBootstrap(ConstCiphertext<DCRTPoly>& ct,
                                       uint32_t numIterations,
                                       uint32_t precision) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalBootstrap(ct, numIterations, precision);
    }
    Ciphertext<DCRTPoly> EvalBootstrapStCFirst(ConstCiphertext<DCRTPoly>& ct,
                                               uint32_t numIterations,
                                               uint32_t precision) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalBootstrapStCFirst(ct, numIterations, precision);
    }
    Ciphertext<DCRTPoly> EvalFBT(ConstCiphertext<DCRTPoly>& ct,
                                  const std::vector<std::complex<double>>& coeffs,
                                  uint32_t digitBitSize,
                                  const BigInteger& initialScaling,
                                  uint64_t postScaling, uint32_t levelToReduce,
                                  size_t order) override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalFBT(ct, coeffs, digitBitSize, initialScaling,
                               postScaling, levelToReduce, order);
    }
    Ciphertext<DCRTPoly> EvalFBT(ConstCiphertext<DCRTPoly>& ct,
                                  const std::vector<int64_t>& coeffs,
                                  uint32_t digitBitSize,
                                  const BigInteger& initialScaling,
                                  uint64_t postScaling, uint32_t levelToReduce,
                                  size_t order) override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalFBT(ct, coeffs, digitBitSize, initialScaling,
                               postScaling, levelToReduce, order);
    }
    Ciphertext<DCRTPoly> EvalFBTNoDecoding(ConstCiphertext<DCRTPoly>& ct,
                                            const std::vector<std::complex<double>>& coeffs,
                                            uint32_t digitBitSize,
                                            const BigInteger& initialScaling,
                                            size_t order) override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalFBTNoDecoding(ct, coeffs, digitBitSize, initialScaling, order);
    }
    Ciphertext<DCRTPoly> EvalFBTNoDecoding(ConstCiphertext<DCRTPoly>& ct,
                                            const std::vector<int64_t>& coeffs,
                                            uint32_t digitBitSize,
                                            const BigInteger& initialScaling,
                                            size_t order) override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalFBTNoDecoding(ct, coeffs, digitBitSize, initialScaling, order);
    }
    Ciphertext<DCRTPoly> EvalHomDecoding(ConstCiphertext<DCRTPoly>& ct,
                                          uint64_t postScaling,
                                          uint32_t levelToReduce) override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalHomDecoding(ct, postScaling, levelToReduce);
    }
    std::shared_ptr<seriesPowers<DCRTPoly>> EvalMVBPrecompute(
        ConstCiphertext<DCRTPoly>& ct,
        const std::vector<std::complex<double>>& coeffs,
        uint32_t digitBitSize, const BigInteger& initialScaling,
        size_t order) override {
        if (g_replay_mode) return nullptr;
        return m_real->EvalMVBPrecompute(ct, coeffs, digitBitSize, initialScaling, order);
    }
    std::shared_ptr<seriesPowers<DCRTPoly>> EvalMVBPrecompute(
        ConstCiphertext<DCRTPoly>& ct,
        const std::vector<int64_t>& coeffs,
        uint32_t digitBitSize, const BigInteger& initialScaling,
        size_t order) override {
        if (g_replay_mode) return nullptr;
        return m_real->EvalMVBPrecompute(ct, coeffs, digitBitSize, initialScaling, order);
    }
    Ciphertext<DCRTPoly> EvalMVB(
        const std::shared_ptr<seriesPowers<DCRTPoly>> cts,
        const std::vector<std::complex<double>>& coeffs,
        uint32_t digitBitSize, const uint64_t postScaling,
        uint32_t levelToReduce, size_t order) override {
        if (g_replay_mode) return dummy_from_cc();
        return m_real->EvalMVB(cts, coeffs, digitBitSize, postScaling, levelToReduce, order);
    }
    Ciphertext<DCRTPoly> EvalMVB(
        const std::shared_ptr<seriesPowers<DCRTPoly>> cts,
        const std::vector<int64_t>& coeffs,
        uint32_t digitBitSize, const uint64_t postScaling,
        uint32_t levelToReduce, size_t order) override {
        if (g_replay_mode) return dummy_from_cc();
        return m_real->EvalMVB(cts, coeffs, digitBitSize, postScaling, levelToReduce, order);
    }
    Ciphertext<DCRTPoly> EvalMVBNoDecoding(
        const std::shared_ptr<seriesPowers<DCRTPoly>> cts,
        const std::vector<std::complex<double>>& coeffs,
        uint32_t digitBitSize, size_t order) override {
        if (g_replay_mode) return dummy_from_cc();
        return m_real->EvalMVBNoDecoding(cts, coeffs, digitBitSize, order);
    }
    Ciphertext<DCRTPoly> EvalMVBNoDecoding(
        const std::shared_ptr<seriesPowers<DCRTPoly>> cts,
        const std::vector<int64_t>& coeffs,
        uint32_t digitBitSize, size_t order) override {
        if (g_replay_mode) return dummy_from_cc();
        return m_real->EvalMVBNoDecoding(cts, coeffs, digitBitSize, order);
    }
    Ciphertext<DCRTPoly> EvalHermiteTrigSeries(
        ConstCiphertext<DCRTPoly>& ct,
        const std::vector<std::complex<double>>& coeffsCheb,
        double a, double b,
        const std::vector<std::complex<double>>& coeffsHerm,
        size_t precomp) override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalHermiteTrigSeries(ct, coeffsCheb, a, b, coeffsHerm, precomp);
    }
    Ciphertext<DCRTPoly> EvalHermiteTrigSeries(
        ConstCiphertext<DCRTPoly>& ct,
        const std::vector<std::complex<double>>& coeffsCheb,
        double a, double b,
        const std::vector<int64_t>& coeffsHerm,
        size_t precomp) override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalHermiteTrigSeries(ct, coeffsCheb, a, b, coeffsHerm, precomp);
    }

    // Getters/setters: always forward
    uint32_t GetCKKSBootCorrectionFactor() const override {
        return m_real->GetCKKSBootCorrectionFactor();
    }
    void SetCKKSBootCorrectionFactor(uint32_t cf) override {
        m_real->SetCKKSBootCorrectionFactor(cf);
    }

};

// ---------------------------------------------------------------------------
// NiobiumAutoAdvancedSHE — proxy for AdvancedSHEBase<DCRTPoly>
//
// Intercepts polynomial-evaluation and linear-weighted-sum operations that
// SchemeBase forwards to m_AdvancedSHE via non-virtual methods (so they
// cannot be caught by NiobiumAutoScheme's virtual overrides alone).
// ---------------------------------------------------------------------------

class NiobiumAutoAdvancedSHE final : public AdvancedSHEBase<DCRTPoly> {
    std::shared_ptr<AdvancedSHEBase<DCRTPoly>> m_real;
    std::weak_ptr<CryptoContextImpl<DCRTPoly>> m_cc;

    // Dummy helpers — const-ref overload handles both by-ref and by-value callers
    static Ciphertext<DCRTPoly> dummy(const ConstCiphertext<DCRTPoly>& ct) {
        return std::make_shared<CiphertextImpl<DCRTPoly>>(
            ct->GetCryptoContext(), ct->GetKeyTag(), ct->GetEncodingType());
    }
    static Ciphertext<DCRTPoly> dummy(const std::vector<Ciphertext<DCRTPoly>>& cts) {
        return std::make_shared<CiphertextImpl<DCRTPoly>>(
            cts[0]->GetCryptoContext(), cts[0]->GetKeyTag(), cts[0]->GetEncodingType());
    }
    static Ciphertext<DCRTPoly> dummy(std::vector<Ciphertext<DCRTPoly>>& cts) {
        return std::make_shared<CiphertextImpl<DCRTPoly>>(
            cts[0]->GetCryptoContext(), cts[0]->GetKeyTag(), cts[0]->GetEncodingType());
    }
    static Ciphertext<DCRTPoly> dummy(std::vector<ReadOnlyCiphertext<DCRTPoly>>& cts) {
        return std::make_shared<CiphertextImpl<DCRTPoly>>(
            cts[0]->GetCryptoContext(), cts[0]->GetKeyTag(), cts[0]->GetEncodingType());
    }
    Ciphertext<DCRTPoly> dummy_from_cc() const {
        auto cc = m_cc.lock();
        return cc ? std::make_shared<CiphertextImpl<DCRTPoly>>(cc) : nullptr;
    }

public:
    NiobiumAutoAdvancedSHE(std::shared_ptr<AdvancedSHEBase<DCRTPoly>> real,
                           std::weak_ptr<CryptoContextImpl<DCRTPoly>> cc)
        : m_real(std::move(real)), m_cc(std::move(cc)) {}

    // -------------------------------------------------------------------------
    // EvalAddMany / EvalAddManyInPlace / EvalMultMany
    // -------------------------------------------------------------------------
    Ciphertext<DCRTPoly> EvalAddMany(
        const std::vector<Ciphertext<DCRTPoly>>& cts) const override {
        if (g_replay_mode) return dummy(cts);
        return m_real->EvalAddMany(cts);
    }
    Ciphertext<DCRTPoly> EvalAddManyInPlace(
        std::vector<Ciphertext<DCRTPoly>>& cts) const override {
        if (g_replay_mode) return dummy(cts);
        return m_real->EvalAddManyInPlace(cts);
    }
    Ciphertext<DCRTPoly> EvalMultMany(
        const std::vector<Ciphertext<DCRTPoly>>& cts,
        const std::vector<EvalKey<DCRTPoly>>& ekVec) const override {
        if (g_replay_mode) return dummy(cts);
        return m_real->EvalMultMany(cts, ekVec);
    }

    // -------------------------------------------------------------------------
    // Linear weighted sum
    // -------------------------------------------------------------------------
    Ciphertext<DCRTPoly> EvalLinearWSum(
        std::vector<ReadOnlyCiphertext<DCRTPoly>>& cts,
        const std::vector<int64_t>& w) const override {
        if (g_replay_mode) return dummy(cts);
        return m_real->EvalLinearWSum(cts, w);
    }
    Ciphertext<DCRTPoly> EvalLinearWSum(
        std::vector<ReadOnlyCiphertext<DCRTPoly>>& cts,
        const std::vector<double>& w) const override {
        if (g_replay_mode) return dummy(cts);
        return m_real->EvalLinearWSum(cts, w);
    }
    Ciphertext<DCRTPoly> EvalLinearWSum(
        std::vector<ReadOnlyCiphertext<DCRTPoly>>& cts,
        const std::vector<std::complex<double>>& w) const override {
        if (g_replay_mode) return dummy(cts);
        return m_real->EvalLinearWSum(cts, w);
    }
    Ciphertext<DCRTPoly> EvalLinearWSumMutable(
        std::vector<Ciphertext<DCRTPoly>>& cts,
        const std::vector<int64_t>& w) const override {
        if (g_replay_mode) return dummy(cts);
        return m_real->EvalLinearWSumMutable(cts, w);
    }
    Ciphertext<DCRTPoly> EvalLinearWSumMutable(
        std::vector<Ciphertext<DCRTPoly>>& cts,
        const std::vector<double>& w) const override {
        if (g_replay_mode) return dummy(cts);
        return m_real->EvalLinearWSumMutable(cts, w);
    }
    Ciphertext<DCRTPoly> EvalLinearWSumMutable(
        std::vector<Ciphertext<DCRTPoly>>& cts,
        const std::vector<std::complex<double>>& w) const override {
        if (g_replay_mode) return dummy(cts);
        return m_real->EvalLinearWSumMutable(cts, w);
    }

    // -------------------------------------------------------------------------
    // EvalPowers precompute — return nullptr in replay (no polynomial work)
    // -------------------------------------------------------------------------
    std::shared_ptr<seriesPowers<DCRTPoly>> EvalPowers(
        ConstCiphertext<DCRTPoly>& ct,
        const std::vector<int64_t>& c) const override {
        if (g_replay_mode) return nullptr;
        return m_real->EvalPowers(ct, c);
    }
    std::shared_ptr<seriesPowers<DCRTPoly>> EvalPowers(
        ConstCiphertext<DCRTPoly>& ct,
        const std::vector<double>& c) const override {
        if (g_replay_mode) return nullptr;
        return m_real->EvalPowers(ct, c);
    }
    std::shared_ptr<seriesPowers<DCRTPoly>> EvalPowers(
        ConstCiphertext<DCRTPoly>& ct,
        const std::vector<std::complex<double>>& c) const override {
        if (g_replay_mode) return nullptr;
        return m_real->EvalPowers(ct, c);
    }

    // -------------------------------------------------------------------------
    // EvalPoly
    // -------------------------------------------------------------------------
    Ciphertext<DCRTPoly> EvalPoly(
        ConstCiphertext<DCRTPoly>& ct,
        const std::vector<int64_t>& c) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalPoly(ct, c);
    }
    Ciphertext<DCRTPoly> EvalPoly(
        ConstCiphertext<DCRTPoly>& ct,
        const std::vector<double>& c) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalPoly(ct, c);
    }
    Ciphertext<DCRTPoly> EvalPoly(
        ConstCiphertext<DCRTPoly>& ct,
        const std::vector<std::complex<double>>& c) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalPoly(ct, c);
    }
    Ciphertext<DCRTPoly> EvalPolyWithPrecomp(
        std::shared_ptr<seriesPowers<DCRTPoly>> p,
        const std::vector<int64_t>& c) const override {
        if (g_replay_mode) return dummy_from_cc();
        return m_real->EvalPolyWithPrecomp(p, c);
    }
    Ciphertext<DCRTPoly> EvalPolyWithPrecomp(
        std::shared_ptr<seriesPowers<DCRTPoly>> p,
        const std::vector<double>& c) const override {
        if (g_replay_mode) return dummy_from_cc();
        return m_real->EvalPolyWithPrecomp(p, c);
    }
    Ciphertext<DCRTPoly> EvalPolyWithPrecomp(
        std::shared_ptr<seriesPowers<DCRTPoly>> p,
        const std::vector<std::complex<double>>& c) const override {
        if (g_replay_mode) return dummy_from_cc();
        return m_real->EvalPolyWithPrecomp(p, c);
    }
    Ciphertext<DCRTPoly> EvalPolyLinear(
        ConstCiphertext<DCRTPoly>& ct,
        const std::vector<int64_t>& c) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalPolyLinear(ct, c);
    }
    Ciphertext<DCRTPoly> EvalPolyLinear(
        ConstCiphertext<DCRTPoly>& ct,
        const std::vector<double>& c) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalPolyLinear(ct, c);
    }
    Ciphertext<DCRTPoly> EvalPolyLinear(
        ConstCiphertext<DCRTPoly>& ct,
        const std::vector<std::complex<double>>& c) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalPolyLinear(ct, c);
    }
    Ciphertext<DCRTPoly> EvalPolyPS(
        ConstCiphertext<DCRTPoly>& ct,
        const std::vector<int64_t>& c) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalPolyPS(ct, c);
    }
    Ciphertext<DCRTPoly> EvalPolyPS(
        ConstCiphertext<DCRTPoly>& ct,
        const std::vector<double>& c) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalPolyPS(ct, c);
    }
    Ciphertext<DCRTPoly> EvalPolyPS(
        ConstCiphertext<DCRTPoly>& ct,
        const std::vector<std::complex<double>>& c) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalPolyPS(ct, c);
    }

    // -------------------------------------------------------------------------
    // EvalChebyshev series
    // -------------------------------------------------------------------------
    std::shared_ptr<seriesPowers<DCRTPoly>> EvalChebyPolys(
        ConstCiphertext<DCRTPoly>& ct,
        const std::vector<int64_t>& c, double a, double b) const override {
        if (g_replay_mode) return nullptr;
        return m_real->EvalChebyPolys(ct, c, a, b);
    }
    std::shared_ptr<seriesPowers<DCRTPoly>> EvalChebyPolys(
        ConstCiphertext<DCRTPoly>& ct,
        const std::vector<double>& c, double a, double b) const override {
        if (g_replay_mode) return nullptr;
        return m_real->EvalChebyPolys(ct, c, a, b);
    }
    std::shared_ptr<seriesPowers<DCRTPoly>> EvalChebyPolys(
        ConstCiphertext<DCRTPoly>& ct,
        const std::vector<std::complex<double>>& c, double a, double b) const override {
        if (g_replay_mode) return nullptr;
        return m_real->EvalChebyPolys(ct, c, a, b);
    }
    Ciphertext<DCRTPoly> EvalChebyshevSeries(
        ConstCiphertext<DCRTPoly>& ct,
        const std::vector<int64_t>& c, double a, double b) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalChebyshevSeries(ct, c, a, b);
    }
    Ciphertext<DCRTPoly> EvalChebyshevSeries(
        ConstCiphertext<DCRTPoly>& ct,
        const std::vector<double>& c, double a, double b) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalChebyshevSeries(ct, c, a, b);
    }
    Ciphertext<DCRTPoly> EvalChebyshevSeries(
        ConstCiphertext<DCRTPoly>& ct,
        const std::vector<std::complex<double>>& c, double a, double b) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalChebyshevSeries(ct, c, a, b);
    }
    Ciphertext<DCRTPoly> EvalChebyshevSeriesWithPrecomp(
        std::shared_ptr<seriesPowers<DCRTPoly>> p,
        const std::vector<int64_t>& c) const override {
        if (g_replay_mode) return dummy_from_cc();
        return m_real->EvalChebyshevSeriesWithPrecomp(p, c);
    }
    Ciphertext<DCRTPoly> EvalChebyshevSeriesWithPrecomp(
        std::shared_ptr<seriesPowers<DCRTPoly>> p,
        const std::vector<double>& c) const override {
        if (g_replay_mode) return dummy_from_cc();
        return m_real->EvalChebyshevSeriesWithPrecomp(p, c);
    }
    Ciphertext<DCRTPoly> EvalChebyshevSeriesWithPrecomp(
        std::shared_ptr<seriesPowers<DCRTPoly>> p,
        const std::vector<std::complex<double>>& c) const override {
        if (g_replay_mode) return dummy_from_cc();
        return m_real->EvalChebyshevSeriesWithPrecomp(p, c);
    }
    Ciphertext<DCRTPoly> EvalChebyshevSeriesLinear(
        ConstCiphertext<DCRTPoly>& ct,
        const std::vector<int64_t>& c, double a, double b) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalChebyshevSeriesLinear(ct, c, a, b);
    }
    Ciphertext<DCRTPoly> EvalChebyshevSeriesLinear(
        ConstCiphertext<DCRTPoly>& ct,
        const std::vector<double>& c, double a, double b) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalChebyshevSeriesLinear(ct, c, a, b);
    }
    Ciphertext<DCRTPoly> EvalChebyshevSeriesLinear(
        ConstCiphertext<DCRTPoly>& ct,
        const std::vector<std::complex<double>>& c, double a, double b) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalChebyshevSeriesLinear(ct, c, a, b);
    }
    Ciphertext<DCRTPoly> EvalChebyshevSeriesPS(
        ConstCiphertext<DCRTPoly>& ct,
        const std::vector<int64_t>& c, double a, double b) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalChebyshevSeriesPS(ct, c, a, b);
    }
    Ciphertext<DCRTPoly> EvalChebyshevSeriesPS(
        ConstCiphertext<DCRTPoly>& ct,
        const std::vector<double>& c, double a, double b) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalChebyshevSeriesPS(ct, c, a, b);
    }
    Ciphertext<DCRTPoly> EvalChebyshevSeriesPS(
        ConstCiphertext<DCRTPoly>& ct,
        const std::vector<std::complex<double>>& c, double a, double b) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalChebyshevSeriesPS(ct, c, a, b);
    }

    // -------------------------------------------------------------------------
    // Sum key-gen: always forward with pause
    // -------------------------------------------------------------------------
    std::shared_ptr<std::map<uint32_t, EvalKey<DCRTPoly>>> EvalSumKeyGen(
        const PrivateKey<DCRTPoly> sk) const override {
        niobium::ScopedPause pause;
        return m_real->EvalSumKeyGen(sk);
    }
    std::shared_ptr<std::map<uint32_t, EvalKey<DCRTPoly>>> EvalSumRowsKeyGen(
        const PrivateKey<DCRTPoly> sk, uint32_t rowSize, uint32_t subringDim,
        std::vector<uint32_t>& indices) const override {
        niobium::ScopedPause pause;
        return m_real->EvalSumRowsKeyGen(sk, rowSize, subringDim, indices);
    }
    std::shared_ptr<std::map<uint32_t, EvalKey<DCRTPoly>>> EvalSumColsKeyGen(
        const PrivateKey<DCRTPoly> sk,
        std::vector<uint32_t>& indices) const override {
        niobium::ScopedPause pause;
        return m_real->EvalSumColsKeyGen(sk, indices);
    }

    // -------------------------------------------------------------------------
    // Sum compute ops
    // -------------------------------------------------------------------------
    Ciphertext<DCRTPoly> EvalSum(
        ConstCiphertext<DCRTPoly> ct, uint32_t batchSize,
        const std::map<uint32_t, EvalKey<DCRTPoly>>& keyMap) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalSum(ct, batchSize, keyMap);
    }
    Ciphertext<DCRTPoly> EvalSumRows(
        ConstCiphertext<DCRTPoly> ct, uint32_t numRows,
        const std::map<uint32_t, EvalKey<DCRTPoly>>& keyMap,
        uint32_t subringDim) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalSumRows(ct, numRows, keyMap, subringDim);
    }
    Ciphertext<DCRTPoly> EvalSumCols(
        ConstCiphertext<DCRTPoly> ct, uint32_t numCols,
        const std::map<uint32_t, EvalKey<DCRTPoly>>& keyMap,
        const std::map<uint32_t, EvalKey<DCRTPoly>>& rightKeyMap) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalSumCols(ct, numCols, keyMap, rightKeyMap);
    }
    Ciphertext<DCRTPoly> EvalInnerProduct(
        ConstCiphertext<DCRTPoly> ct1, ConstCiphertext<DCRTPoly> ct2,
        uint32_t batchSize,
        const std::map<uint32_t, EvalKey<DCRTPoly>>& sumKeyMap,
        const EvalKey<DCRTPoly> multKey) const override {
        if (g_replay_mode) return dummy(ct1);
        return m_real->EvalInnerProduct(ct1, ct2, batchSize, sumKeyMap, multKey);
    }
    Ciphertext<DCRTPoly> EvalInnerProduct(
        ConstCiphertext<DCRTPoly> ct, ConstPlaintext pt,
        uint32_t batchSize,
        const std::map<uint32_t, EvalKey<DCRTPoly>>& sumKeyMap) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalInnerProduct(ct, pt, batchSize, sumKeyMap);
    }
    Ciphertext<DCRTPoly> AddRandomNoise(ConstCiphertext<DCRTPoly> ct) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->AddRandomNoise(ct);
    }
    Ciphertext<DCRTPoly> EvalMerge(
        const std::vector<Ciphertext<DCRTPoly>>& cts,
        const std::map<uint32_t, EvalKey<DCRTPoly>>& keyMap) const override {
        if (g_replay_mode) return dummy(cts);
        return m_real->EvalMerge(cts, keyMap);
    }
};

// ---------------------------------------------------------------------------
// NiobiumAutoScheme — proxy for SchemeBase<DCRTPoly>
// ---------------------------------------------------------------------------

class NiobiumAutoScheme final : public SchemeBase<DCRTPoly> {
    std::shared_ptr<SchemeBase<DCRTPoly>> m_real;

    static Ciphertext<DCRTPoly> dummy(ConstCiphertext<DCRTPoly>& ct) {
        g_replay_noop_count.fetch_add(1, std::memory_order_relaxed);
        return std::make_shared<CiphertextImpl<DCRTPoly>>(
            ct->GetCryptoContext(), ct->GetKeyTag(), ct->GetEncodingType());
    }
    static Ciphertext<DCRTPoly> dummy(const Ciphertext<DCRTPoly>& ct) {
        g_replay_noop_count.fetch_add(1, std::memory_order_relaxed);
        return std::make_shared<CiphertextImpl<DCRTPoly>>(
            ct->GetCryptoContext(), ct->GetKeyTag(), ct->GetEncodingType());
    }
    static std::shared_ptr<std::vector<DCRTPoly>> empty_vec() {
        return std::make_shared<std::vector<DCRTPoly>>();
    }

public:
    // Returns the wrapped real scheme (e.g. SchemeCKKSRNS) for serialization.
    std::shared_ptr<SchemeBase<DCRTPoly>> GetRealScheme() const { return m_real; }

    NiobiumAutoScheme(std::shared_ptr<SchemeBase<DCRTPoly>> real,
                      std::weak_ptr<CryptoContextImpl<DCRTPoly>> cc)
        : m_real(std::move(real)) {
        // Copy sub-scheme pointers into our SchemeBase base so that non-virtual
        // methods (VerifyXxx, KeySwitchDownFirstElement, etc.) work correctly.
        // These members are accessed directly by SchemeBase methods that we do
        // not override — they must point to the real implementations.
        m_PKE        = m_real->GetPKE();
        m_KeySwitch  = m_real->GetKeySwitch();
        m_LeveledSHE = m_real->GetLeveledSHE();
        m_Multiparty = m_real->GetMultiparty();
        // m_AdvancedSHE is wrapped so EvalPoly/EvalChebyshev/EvalLinearWSum
        // operations are intercepted (non-virtual in SchemeBase, so virtual
        // override alone is insufficient).
        if (auto adv = m_real->GetAdvancedSHE())
            m_AdvancedSHE = std::make_shared<NiobiumAutoAdvancedSHE>(adv, cc);
        // m_FHE is wrapped so EvalBootstrap is intercepted
        if (auto fhe = m_real->GetFHE())
            SetFHE(std::make_shared<NiobiumAutoFHE>(fhe, cc));
    }

    // -------------------------------------------------------------------------
    // Non-compute: always forward
    // -------------------------------------------------------------------------

    bool operator==(const SchemeBase<DCRTPoly>& s) const override { return m_real->operator==(s); }
    bool operator!=(const SchemeBase<DCRTPoly>& s) const override { return m_real->operator!=(s); }
    void Enable(PKESchemeFeature f) override { m_real->Enable(f); }
    std::string SerializedObjectName() const override { return m_real->SerializedObjectName(); }

    // -------------------------------------------------------------------------
    // PKE — key gen and encrypt always forward; decrypt always forward
    // -------------------------------------------------------------------------

    KeyPair<DCRTPoly> KeyGen(CryptoContext<DCRTPoly> cc, bool sparse) const override {
        niobium::ScopedPause pause;
        return m_real->KeyGen(cc, sparse);
    }
    Ciphertext<DCRTPoly> Encrypt(const DCRTPoly& pt, const PrivateKey<DCRTPoly> sk) const override {
        niobium::ScopedPause pause;
        return m_real->Encrypt(pt, sk);
    }
    Ciphertext<DCRTPoly> Encrypt(const DCRTPoly& pt, const PublicKey<DCRTPoly> pk) const override {
        niobium::ScopedPause pause;
        return m_real->Encrypt(pt, pk);
    }
    DecryptResult Decrypt(ConstCiphertext<DCRTPoly>& ct, const PrivateKey<DCRTPoly> sk,
                          NativePoly* pt) const override {
        niobium::ScopedPause pause;
        return m_real->Decrypt(ct, sk, pt);
    }
    DecryptResult Decrypt(ConstCiphertext<DCRTPoly>& ct, const PrivateKey<DCRTPoly> sk,
                          Poly* pt) const override {
        niobium::ScopedPause pause;
        return m_real->Decrypt(ct, sk, pt);
    }

    // -------------------------------------------------------------------------
    // Key switch — gen always forward; switch ops short-circuit in replay
    // -------------------------------------------------------------------------

    EvalKey<DCRTPoly> KeySwitchGen(const PrivateKey<DCRTPoly> oldSk,
                                   const PrivateKey<DCRTPoly> newSk) const override {
        niobium::ScopedPause pause;
        return m_real->KeySwitchGen(oldSk, newSk);
    }
    EvalKey<DCRTPoly> KeySwitchGen(const PrivateKey<DCRTPoly> oldSk,
                                   const PrivateKey<DCRTPoly> newSk,
                                   const EvalKey<DCRTPoly> ek) const override {
        niobium::ScopedPause pause;
        return m_real->KeySwitchGen(oldSk, newSk, ek);
    }
    EvalKey<DCRTPoly> KeySwitchGen(const PrivateKey<DCRTPoly> oldSk,
                                   const PublicKey<DCRTPoly> newPk) const override {
        niobium::ScopedPause pause;
        return m_real->KeySwitchGen(oldSk, newPk);
    }
    Ciphertext<DCRTPoly> KeySwitch(ConstCiphertext<DCRTPoly>& ct,
                                   const EvalKey<DCRTPoly> ek) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->KeySwitch(ct, ek);
    }
    void KeySwitchInPlace(Ciphertext<DCRTPoly>& ct,
                          const EvalKey<DCRTPoly> ek) const override {
        if (g_replay_mode) return;
        m_real->KeySwitchInPlace(ct, ek);
    }
    Ciphertext<DCRTPoly> KeySwitchDown(ConstCiphertext<DCRTPoly>& ct) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->KeySwitchDown(ct);
    }
    Ciphertext<DCRTPoly> KeySwitchExt(ConstCiphertext<DCRTPoly>& ct,
                                      bool addFirst) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->KeySwitchExt(ct, addFirst);
    }
    std::vector<DCRTPoly> KeySwitchCore(
        const DCRTPoly& a, const EvalKey<DCRTPoly> ek) const override {
        if (g_replay_mode) return {};
        return m_real->KeySwitchCore(a, ek);
    }
    std::shared_ptr<std::vector<DCRTPoly>> EvalKeySwitchPrecomputeCore(
        const DCRTPoly& c,
        std::shared_ptr<CryptoParametersBase<DCRTPoly>> cp) const override {
        if (g_replay_mode) return empty_vec();
        return m_real->EvalKeySwitchPrecomputeCore(c, cp);
    }
    std::vector<DCRTPoly> EvalFastKeySwitchCoreExt(
        const std::shared_ptr<std::vector<DCRTPoly>> digits,
        const EvalKey<DCRTPoly> ek,
        const std::shared_ptr<typename DCRTPoly::Params> params) const override {
        if (g_replay_mode) return {};
        return m_real->EvalFastKeySwitchCoreExt(digits, ek, params);
    }
    std::vector<DCRTPoly> EvalFastKeySwitchCore(
        const std::shared_ptr<std::vector<DCRTPoly>> digits,
        const EvalKey<DCRTPoly> ek,
        const std::shared_ptr<typename DCRTPoly::Params> params) const override {
        if (g_replay_mode) return {};
        return m_real->EvalFastKeySwitchCore(digits, ek, params);
    }

    // -------------------------------------------------------------------------
    // PRE
    // -------------------------------------------------------------------------

    EvalKey<DCRTPoly> ReKeyGen(const PrivateKey<DCRTPoly> oldSk,
                               const PublicKey<DCRTPoly> newPk) const override {
        niobium::ScopedPause pause;
        return m_real->ReKeyGen(oldSk, newPk);
    }
    Ciphertext<DCRTPoly> ReEncrypt(ConstCiphertext<DCRTPoly>& ct,
                                   const EvalKey<DCRTPoly> ek,
                                   const PublicKey<DCRTPoly> pk) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->ReEncrypt(ct, ek, pk);
    }

    // -------------------------------------------------------------------------
    // EvalNegate
    // -------------------------------------------------------------------------

    Ciphertext<DCRTPoly> EvalNegate(ConstCiphertext<DCRTPoly>& ct) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalNegate(ct);
    }
    void EvalNegateInPlace(Ciphertext<DCRTPoly>& ct) const override {
        if (g_replay_mode) return;
        m_real->EvalNegateInPlace(ct);
    }

    // -------------------------------------------------------------------------
    // EvalAdd
    // -------------------------------------------------------------------------

    Ciphertext<DCRTPoly> EvalAdd(ConstCiphertext<DCRTPoly>& ct1,
                                 ConstCiphertext<DCRTPoly>& ct2) const override {
        if (g_replay_mode) return dummy(ct1);
        return m_real->EvalAdd(ct1, ct2);
    }
    void EvalAddInPlace(Ciphertext<DCRTPoly>& ct1,
                        ConstCiphertext<DCRTPoly>& ct2) const override {
        if (g_replay_mode) return;
        m_real->EvalAddInPlace(ct1, ct2);
    }
    Ciphertext<DCRTPoly> EvalAddMutable(Ciphertext<DCRTPoly>& ct1,
                                        Ciphertext<DCRTPoly>& ct2) const override {
        if (g_replay_mode) return dummy(ct1);
        return m_real->EvalAddMutable(ct1, ct2);
    }
    void EvalAddMutableInPlace(Ciphertext<DCRTPoly>& ct1,
                               Ciphertext<DCRTPoly>& ct2) const override {
        if (g_replay_mode) return;
        m_real->EvalAddMutableInPlace(ct1, ct2);
    }
    Ciphertext<DCRTPoly> EvalAdd(ConstCiphertext<DCRTPoly>& ct,
                                 ConstPlaintext& pt) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalAdd(ct, pt);
    }
    void EvalAddInPlace(Ciphertext<DCRTPoly>& ct,
                        ConstPlaintext& pt) const override {
        if (g_replay_mode) return;
        m_real->EvalAddInPlace(ct, pt);
    }
    Ciphertext<DCRTPoly> EvalAddMutable(Ciphertext<DCRTPoly>& ct,
                                        Plaintext& pt) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalAddMutable(ct, pt);
    }
    void EvalAddInPlace(Ciphertext<DCRTPoly>& ct,
                        NativeInteger c) const override {
        if (g_replay_mode) return;
        m_real->EvalAddInPlace(ct, c);
    }
    Ciphertext<DCRTPoly> EvalAdd(ConstCiphertext<DCRTPoly>& ct,
                                 double c) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalAdd(ct, c);
    }
    void EvalAddInPlace(Ciphertext<DCRTPoly>& ct, double c) const override {
        if (g_replay_mode) return;
        m_real->EvalAddInPlace(ct, c);
    }
    Ciphertext<DCRTPoly> EvalAdd(ConstCiphertext<DCRTPoly>& ct,
                                 std::complex<double> c) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalAdd(ct, c);
    }
    void EvalAddInPlace(Ciphertext<DCRTPoly>& ct,
                        std::complex<double> c) const override {
        if (g_replay_mode) return;
        m_real->EvalAddInPlace(ct, c);
    }

    // -------------------------------------------------------------------------
    // EvalSub
    // -------------------------------------------------------------------------

    Ciphertext<DCRTPoly> EvalSub(ConstCiphertext<DCRTPoly>& ct1,
                                 ConstCiphertext<DCRTPoly>& ct2) const override {
        if (g_replay_mode) return dummy(ct1);
        return m_real->EvalSub(ct1, ct2);
    }
    void EvalSubInPlace(Ciphertext<DCRTPoly>& ct1,
                        ConstCiphertext<DCRTPoly>& ct2) const override {
        if (g_replay_mode) return;
        m_real->EvalSubInPlace(ct1, ct2);
    }
    Ciphertext<DCRTPoly> EvalSubMutable(Ciphertext<DCRTPoly>& ct1,
                                        Ciphertext<DCRTPoly>& ct2) const override {
        if (g_replay_mode) return dummy(ct1);
        return m_real->EvalSubMutable(ct1, ct2);
    }
    void EvalSubMutableInPlace(Ciphertext<DCRTPoly>& ct1,
                               Ciphertext<DCRTPoly>& ct2) const override {
        if (g_replay_mode) return;
        m_real->EvalSubMutableInPlace(ct1, ct2);
    }
    Ciphertext<DCRTPoly> EvalSub(ConstCiphertext<DCRTPoly>& ct,
                                 ConstPlaintext& pt) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalSub(ct, pt);
    }
    void EvalSubInPlace(Ciphertext<DCRTPoly>& ct,
                        ConstPlaintext& pt) const override {
        if (g_replay_mode) return;
        m_real->EvalSubInPlace(ct, pt);
    }
    Ciphertext<DCRTPoly> EvalSubMutable(Ciphertext<DCRTPoly>& ct,
                                        Plaintext& pt) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalSubMutable(ct, pt);
    }
    Ciphertext<DCRTPoly> EvalSub(ConstCiphertext<DCRTPoly>& ct,
                                 NativeInteger c) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalSub(ct, c);
    }
    void EvalSubInPlace(Ciphertext<DCRTPoly>& ct,
                        NativeInteger c) const override {
        if (g_replay_mode) return;
        m_real->EvalSubInPlace(ct, c);
    }
    Ciphertext<DCRTPoly> EvalSub(ConstCiphertext<DCRTPoly>& ct,
                                 double c) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalSub(ct, c);
    }
    void EvalSubInPlace(Ciphertext<DCRTPoly>& ct, double c) const override {
        if (g_replay_mode) return;
        m_real->EvalSubInPlace(ct, c);
    }

    // -------------------------------------------------------------------------
    // EvalMult
    // -------------------------------------------------------------------------

    EvalKey<DCRTPoly> EvalMultKeyGen(const PrivateKey<DCRTPoly> sk) const override {
        niobium::ScopedPause pause;
        return m_real->EvalMultKeyGen(sk);
    }
    std::vector<EvalKey<DCRTPoly>> EvalMultKeysGen(
        const PrivateKey<DCRTPoly> sk) const override {
        niobium::ScopedPause pause;
        return m_real->EvalMultKeysGen(sk);
    }
    Ciphertext<DCRTPoly> EvalMult(ConstCiphertext<DCRTPoly>& ct1,
                                  ConstCiphertext<DCRTPoly>& ct2) const override {
        if (g_replay_mode) return dummy(ct1);
        return m_real->EvalMult(ct1, ct2);
    }
    Ciphertext<DCRTPoly> EvalMultMutable(Ciphertext<DCRTPoly>& ct1,
                                         Ciphertext<DCRTPoly>& ct2) const override {
        if (g_replay_mode) return dummy(ct1);
        return m_real->EvalMultMutable(ct1, ct2);
    }
    Ciphertext<DCRTPoly> EvalSquare(ConstCiphertext<DCRTPoly>& ct) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalSquare(ct);
    }
    Ciphertext<DCRTPoly> EvalSquareMutable(Ciphertext<DCRTPoly>& ct) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalSquareMutable(ct);
    }
    Ciphertext<DCRTPoly> EvalMult(ConstCiphertext<DCRTPoly>& ct1,
                                  ConstCiphertext<DCRTPoly>& ct2,
                                  const EvalKey<DCRTPoly> ek) const override {
        if (g_replay_mode) return dummy(ct1);
        return m_real->EvalMult(ct1, ct2, ek);
    }
    void EvalMultInPlace(Ciphertext<DCRTPoly>& ct1, ConstCiphertext<DCRTPoly>& ct2,
                         const EvalKey<DCRTPoly> ek) const override {
        if (g_replay_mode) return;
        m_real->EvalMultInPlace(ct1, ct2, ek);
    }
    Ciphertext<DCRTPoly> EvalMultMutable(Ciphertext<DCRTPoly>& ct1,
                                         Ciphertext<DCRTPoly>& ct2,
                                         const EvalKey<DCRTPoly> ek) const override {
        if (g_replay_mode) return dummy(ct1);
        return m_real->EvalMultMutable(ct1, ct2, ek);
    }
    void EvalMultMutableInPlace(Ciphertext<DCRTPoly>& ct1,
                                Ciphertext<DCRTPoly>& ct2,
                                const EvalKey<DCRTPoly> ek) const override {
        if (g_replay_mode) return;
        m_real->EvalMultMutableInPlace(ct1, ct2, ek);
    }
    Ciphertext<DCRTPoly> EvalSquare(ConstCiphertext<DCRTPoly>& ct,
                                    const EvalKey<DCRTPoly> ek) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalSquare(ct, ek);
    }
    void EvalSquareInPlace(Ciphertext<DCRTPoly>& ct,
                           const EvalKey<DCRTPoly> ek) const override {
        if (g_replay_mode) return;
        m_real->EvalSquareInPlace(ct, ek);
    }
    Ciphertext<DCRTPoly> EvalSquareMutable(Ciphertext<DCRTPoly>& ct,
                                           const EvalKey<DCRTPoly> ek) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalSquareMutable(ct, ek);
    }
    Ciphertext<DCRTPoly> EvalMultAndRelinearize(
        ConstCiphertext<DCRTPoly>& ct1, ConstCiphertext<DCRTPoly>& ct2,
        const std::vector<EvalKey<DCRTPoly>>& ekVec) const override {
        if (g_replay_mode) return dummy(ct1);
        return m_real->EvalMultAndRelinearize(ct1, ct2, ekVec);
    }
    Ciphertext<DCRTPoly> Relinearize(
        ConstCiphertext<DCRTPoly>& ct,
        const std::vector<EvalKey<DCRTPoly>>& ekVec) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->Relinearize(ct, ekVec);
    }
    void RelinearizeInPlace(Ciphertext<DCRTPoly>& ct,
                            const std::vector<EvalKey<DCRTPoly>>& ekVec) const override {
        if (g_replay_mode) return;
        m_real->RelinearizeInPlace(ct, ekVec);
    }
    Ciphertext<DCRTPoly> EvalMult(ConstCiphertext<DCRTPoly>& ct,
                                  ConstPlaintext& pt) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalMult(ct, pt);
    }
    void EvalMultInPlace(Ciphertext<DCRTPoly>& ct,
                         ConstPlaintext& pt) const override {
        if (g_replay_mode) return;
        m_real->EvalMultInPlace(ct, pt);
    }
    Ciphertext<DCRTPoly> EvalMultMutable(Ciphertext<DCRTPoly>& ct,
                                         Plaintext& pt) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalMultMutable(ct, pt);
    }
    Ciphertext<DCRTPoly> MultByMonomial(ConstCiphertext<DCRTPoly>& ct,
                                        uint32_t power) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->MultByMonomial(ct, power);
    }
    void MultByMonomialInPlace(Ciphertext<DCRTPoly>& ct,
                               uint32_t power) const override {
        if (g_replay_mode) return;
        m_real->MultByMonomialInPlace(ct, power);
    }
    Ciphertext<DCRTPoly> EvalMult(ConstCiphertext<DCRTPoly>& ct,
                                  double c) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalMult(ct, c);
    }
    void EvalMultInPlace(Ciphertext<DCRTPoly>& ct, double c) const override {
        if (g_replay_mode) return;
        m_real->EvalMultInPlace(ct, c);
    }
    Ciphertext<DCRTPoly> EvalMult(ConstCiphertext<DCRTPoly>& ct,
                                  std::complex<double> c) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalMult(ct, c);
    }
    void EvalMultInPlace(Ciphertext<DCRTPoly>& ct,
                         std::complex<double> c) const override {
        if (g_replay_mode) return;
        m_real->EvalMultInPlace(ct, c);
    }
    Ciphertext<DCRTPoly> MultByInteger(ConstCiphertext<DCRTPoly>& ct,
                                       uint64_t i) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->MultByInteger(ct, i);
    }
    void MultByIntegerInPlace(Ciphertext<DCRTPoly>& ct,
                              uint64_t i) const override {
        if (g_replay_mode) return;
        m_real->MultByIntegerInPlace(ct, i);
    }

    // -------------------------------------------------------------------------
    // Automorphism / Rotation
    // -------------------------------------------------------------------------

    std::shared_ptr<std::map<uint32_t, EvalKey<DCRTPoly>>> EvalAutomorphismKeyGen(
        const PrivateKey<DCRTPoly> sk,
        const std::vector<uint32_t>& idxList) const override {
        niobium::ScopedPause pause;
        return m_real->EvalAutomorphismKeyGen(sk, idxList);
    }
    Ciphertext<DCRTPoly> EvalAutomorphism(
        ConstCiphertext<DCRTPoly>& ct, uint32_t i,
        const std::map<uint32_t, EvalKey<DCRTPoly>>& ekMap,
        const char* callerFile = "", const char* callerFunc = "", size_t callerLine = 0) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalAutomorphism(ct, i, ekMap);
    }
    Ciphertext<DCRTPoly> EvalFastRotation(
        ConstCiphertext<DCRTPoly>& ct, const uint32_t index, const uint32_t m,
        const std::shared_ptr<std::vector<DCRTPoly>> precomp) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalFastRotation(ct, index, m, precomp);
    }
    std::shared_ptr<std::vector<DCRTPoly>> EvalFastRotationPrecompute(
        ConstCiphertext<DCRTPoly> ct) const override {
        if (g_replay_mode) return empty_vec();
        return m_real->EvalFastRotationPrecompute(ct);
    }
    Ciphertext<DCRTPoly> EvalFastRotationExt(
        ConstCiphertext<DCRTPoly>& ct, uint32_t index,
        const std::shared_ptr<std::vector<DCRTPoly>> digits, bool addFirst,
        const std::map<uint32_t, EvalKey<DCRTPoly>>& ekMap) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalFastRotationExt(ct, index, digits, addFirst, ekMap);
    }
    std::shared_ptr<std::map<uint32_t, EvalKey<DCRTPoly>>> EvalAtIndexKeyGen(
        const PrivateKey<DCRTPoly> sk,
        const std::vector<int32_t>& idxList) const override {
        niobium::ScopedPause pause;
        return m_real->EvalAtIndexKeyGen(sk, idxList);
    }
    Ciphertext<DCRTPoly> EvalAtIndex(
        ConstCiphertext<DCRTPoly>& ct, uint32_t i,
        const std::map<uint32_t, EvalKey<DCRTPoly>>& ekMap) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalAtIndex(ct, i, ekMap);
    }
    uint32_t FindAutomorphismIndex(uint32_t index, uint32_t m) override {
        return m_real->FindAutomorphismIndex(index, m);
    }

    // -------------------------------------------------------------------------
    // ComposedEvalMult
    // -------------------------------------------------------------------------

    Ciphertext<DCRTPoly> ComposedEvalMult(ConstCiphertext<DCRTPoly>& ct1,
                                          ConstCiphertext<DCRTPoly>& ct2,
                                          const EvalKey<DCRTPoly> ek) const override {
        if (g_replay_mode) return dummy(ct1);
        return m_real->ComposedEvalMult(ct1, ct2, ek);
    }

    // -------------------------------------------------------------------------
    // Level management
    // -------------------------------------------------------------------------

    Ciphertext<DCRTPoly> ModReduce(ConstCiphertext<DCRTPoly>& ct,
                                   size_t levels) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->ModReduce(ct, levels);
    }
    void ModReduceInPlace(Ciphertext<DCRTPoly>& ct, size_t levels) const override {
        if (g_replay_mode) return;
        m_real->ModReduceInPlace(ct, levels);
    }
    Ciphertext<DCRTPoly> ModReduceInternal(ConstCiphertext<DCRTPoly>& ct,
                                           size_t levels) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->ModReduceInternal(ct, levels);
    }
    void ModReduceInternalInPlace(Ciphertext<DCRTPoly>& ct,
                                  size_t levels) const override {
        if (g_replay_mode) return;
        m_real->ModReduceInternalInPlace(ct, levels);
    }
    Ciphertext<DCRTPoly> LevelReduce(ConstCiphertext<DCRTPoly>& ct,
                                     const EvalKey<DCRTPoly> ek,
                                     size_t levels) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->LevelReduce(ct, ek, levels);
    }
    void LevelReduceInPlace(Ciphertext<DCRTPoly>& ct, const EvalKey<DCRTPoly> ek,
                            size_t levels) const override {
        if (g_replay_mode) return;
        m_real->LevelReduceInPlace(ct, ek, levels);
    }
    Ciphertext<DCRTPoly> LevelReduceInternal(ConstCiphertext<DCRTPoly>& ct,
                                             size_t levels) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->LevelReduceInternal(ct, levels);
    }
    void LevelReduceInternalInPlace(Ciphertext<DCRTPoly>& ct,
                                    size_t levels) const override {
        if (g_replay_mode) return;
        m_real->LevelReduceInternalInPlace(ct, levels);
    }
    Ciphertext<DCRTPoly> Compress(ConstCiphertext<DCRTPoly>& ct, size_t towersLeft,
                                  size_t noiseScaleDeg) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->Compress(ct, towersLeft, noiseScaleDeg);
    }
    void AdjustLevelsInPlace(Ciphertext<DCRTPoly>& ct1,
                             Ciphertext<DCRTPoly>& ct2) const override {
        if (g_replay_mode) return;
        m_real->AdjustLevelsInPlace(ct1, ct2);
    }
    void AdjustLevelsAndDepthInPlace(Ciphertext<DCRTPoly>& ct1,
                                     Ciphertext<DCRTPoly>& ct2) const override {
        if (g_replay_mode) return;
        m_real->AdjustLevelsAndDepthInPlace(ct1, ct2);
    }
    void AdjustLevelsAndDepthToOneInPlace(Ciphertext<DCRTPoly>& ct1,
                                          Ciphertext<DCRTPoly>& ct2) const override {
        if (g_replay_mode) return;
        m_real->AdjustLevelsAndDepthToOneInPlace(ct1, ct2);
    }

    // -------------------------------------------------------------------------
    // Advanced SHE
    // -------------------------------------------------------------------------

    Ciphertext<DCRTPoly> EvalAddMany(
        const std::vector<Ciphertext<DCRTPoly>>& cts) const override {
        if (g_replay_mode) return dummy(cts[0]);
        return m_real->EvalAddMany(cts);
    }
    Ciphertext<DCRTPoly> EvalAddManyInPlace(
        std::vector<Ciphertext<DCRTPoly>>& cts) const override {
        if (g_replay_mode) return dummy(cts[0]);
        return m_real->EvalAddManyInPlace(cts);
    }
    Ciphertext<DCRTPoly> EvalMultMany(
        const std::vector<Ciphertext<DCRTPoly>>& cts,
        const std::vector<EvalKey<DCRTPoly>>& ekVec) const override {
        if (g_replay_mode) return dummy(cts[0]);
        return m_real->EvalMultMany(cts, ekVec);
    }

    // -------------------------------------------------------------------------
    // Sum operations
    // -------------------------------------------------------------------------

    std::shared_ptr<std::map<uint32_t, EvalKey<DCRTPoly>>> EvalSumKeyGen(
        const PrivateKey<DCRTPoly> sk) const override {
        niobium::ScopedPause pause;
        return m_real->EvalSumKeyGen(sk);
    }
    std::shared_ptr<std::map<uint32_t, EvalKey<DCRTPoly>>> EvalSumRowsKeyGen(
        const PrivateKey<DCRTPoly> sk, uint32_t rowSize, uint32_t subringDim,
        std::vector<uint32_t>& indices) const override {
        niobium::ScopedPause pause;
        return m_real->EvalSumRowsKeyGen(sk, rowSize, subringDim, indices);
    }
    std::shared_ptr<std::map<uint32_t, EvalKey<DCRTPoly>>> EvalSumColsKeyGen(
        const PrivateKey<DCRTPoly> sk,
        std::vector<uint32_t>& indices) const override {
        niobium::ScopedPause pause;
        return m_real->EvalSumColsKeyGen(sk, indices);
    }
    Ciphertext<DCRTPoly> EvalSum(ConstCiphertext<DCRTPoly> ct, uint32_t batchSize,
                                 const std::map<uint32_t, EvalKey<DCRTPoly>>& evalKeyMap) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalSum(ct, batchSize, evalKeyMap);
    }
    Ciphertext<DCRTPoly> EvalSumRows(
        ConstCiphertext<DCRTPoly>& ct, uint32_t rowSize,
        const std::map<uint32_t, EvalKey<DCRTPoly>>& evalKeyMap,
        uint32_t subringDim) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalSumRows(ct, rowSize, evalKeyMap, subringDim);
    }
    Ciphertext<DCRTPoly> EvalSumCols(
        ConstCiphertext<DCRTPoly>& ct, uint32_t batchSize,
        const std::map<uint32_t, EvalKey<DCRTPoly>>& evalKeyMap,
        const std::map<uint32_t, EvalKey<DCRTPoly>>& rightEvalKeyMap) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalSumCols(ct, batchSize, evalKeyMap, rightEvalKeyMap);
    }
    Ciphertext<DCRTPoly> EvalInnerProduct(ConstCiphertext<DCRTPoly>& ct1,
                                          ConstCiphertext<DCRTPoly>& ct2,
                                          uint32_t batchSize,
                                          const std::map<uint32_t, EvalKey<DCRTPoly>>& evalSumKeyMap,
                                          const EvalKey<DCRTPoly> evalMultKey) const override {
        if (g_replay_mode) return dummy(ct1);
        return m_real->EvalInnerProduct(ct1, ct2, batchSize, evalSumKeyMap, evalMultKey);
    }
    Ciphertext<DCRTPoly> EvalInnerProduct(ConstCiphertext<DCRTPoly>& ct,
                                          ConstPlaintext& pt,
                                          uint32_t batchSize,
                                          const std::map<uint32_t, EvalKey<DCRTPoly>>& evalSumKeyMap) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->EvalInnerProduct(ct, pt, batchSize, evalSumKeyMap);
    }
    Ciphertext<DCRTPoly> AddRandomNoise(
        ConstCiphertext<DCRTPoly> ct) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->AddRandomNoise(ct);
    }
    Ciphertext<DCRTPoly> EvalMerge(const std::vector<Ciphertext<DCRTPoly>>& cts,
                                   const std::map<uint32_t, EvalKey<DCRTPoly>>& evalKeyMap) const override {
        if (g_replay_mode) return dummy(cts[0]);
        return m_real->EvalMerge(cts, evalKeyMap);
    }

    // -------------------------------------------------------------------------
    // Multiparty — key gen always forward; decrypt ops short-circuit
    // -------------------------------------------------------------------------

    KeyPair<DCRTPoly> MultipartyKeyGen(
        CryptoContext<DCRTPoly> cc,
        const std::vector<PrivateKey<DCRTPoly>>& skVec,
        bool makeSparse) override {
        niobium::ScopedPause pause;
        return m_real->MultipartyKeyGen(cc, skVec, makeSparse);
    }
    KeyPair<DCRTPoly> MultipartyKeyGen(CryptoContext<DCRTPoly> cc,
                                       const PublicKey<DCRTPoly> pk,
                                       bool makeSparse, bool fresh) override {
        niobium::ScopedPause pause;
        return m_real->MultipartyKeyGen(cc, pk, makeSparse, fresh);
    }
    Ciphertext<DCRTPoly> MultipartyDecryptMain(
        ConstCiphertext<DCRTPoly>& ct,
        const PrivateKey<DCRTPoly> sk) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->MultipartyDecryptMain(ct, sk);
    }
    Ciphertext<DCRTPoly> MultipartyDecryptLead(
        ConstCiphertext<DCRTPoly>& ct,
        const PrivateKey<DCRTPoly> sk) const override {
        if (g_replay_mode) return dummy(ct);
        return m_real->MultipartyDecryptLead(ct, sk);
    }
    DecryptResult MultipartyDecryptFusion(
        const std::vector<Ciphertext<DCRTPoly>>& cts,
        NativePoly* pt) const override {
        return m_real->MultipartyDecryptFusion(cts, pt);
    }
    DecryptResult MultipartyDecryptFusion(
        const std::vector<Ciphertext<DCRTPoly>>& cts,
        Poly* pt) const override {
        return m_real->MultipartyDecryptFusion(cts, pt);
    }
    EvalKey<DCRTPoly> MultiKeySwitchGen(
        const PrivateKey<DCRTPoly> oldSk, const PrivateKey<DCRTPoly> newSk,
        const EvalKey<DCRTPoly> ek) const override {
        niobium::ScopedPause pause;
        return m_real->MultiKeySwitchGen(oldSk, newSk, ek);
    }
    std::shared_ptr<std::map<uint32_t, EvalKey<DCRTPoly>>> MultiEvalAutomorphismKeyGen(
        const PrivateKey<DCRTPoly> sk,
        const std::shared_ptr<std::map<uint32_t, EvalKey<DCRTPoly>>> evalAutoKeyMap,
        const std::vector<uint32_t>& idxList,
        const std::string& keyId) override {
        niobium::ScopedPause pause;
        return m_real->MultiEvalAutomorphismKeyGen(sk, evalAutoKeyMap, idxList, keyId);
    }
    std::shared_ptr<std::map<uint32_t, EvalKey<DCRTPoly>>> MultiEvalAtIndexKeyGen(
        const PrivateKey<DCRTPoly> sk,
        const std::shared_ptr<std::map<uint32_t, EvalKey<DCRTPoly>>> evalAutoKeyMap,
        const std::vector<int32_t>& idxList,
        const std::string& keyId) override {
        return m_real->MultiEvalAtIndexKeyGen(sk, evalAutoKeyMap, idxList, keyId);
    }
    std::shared_ptr<std::map<uint32_t, EvalKey<DCRTPoly>>> MultiEvalSumKeyGen(
        const PrivateKey<DCRTPoly> sk,
        const std::shared_ptr<std::map<uint32_t, EvalKey<DCRTPoly>>> evalSumKeyMap,
        const std::string& keyId = "") override {
        niobium::ScopedPause pause;
        return m_real->MultiEvalSumKeyGen(sk, evalSumKeyMap, keyId);
    }
    EvalKey<DCRTPoly> MultiAddEvalKeys(EvalKey<DCRTPoly> ek1,
                                       EvalKey<DCRTPoly> ek2,
                                       const std::string& keyTag) override {
        return m_real->MultiAddEvalKeys(ek1, ek2, keyTag);
    }
    EvalKey<DCRTPoly> MultiMultEvalKey(PrivateKey<DCRTPoly> sk,
                                       EvalKey<DCRTPoly> ek,
                                       const std::string& keyTag) override {
        return m_real->MultiMultEvalKey(sk, ek, keyTag);
    }
    std::shared_ptr<std::map<uint32_t, EvalKey<DCRTPoly>>> MultiAddEvalSumKeys(
        const std::shared_ptr<std::map<uint32_t, EvalKey<DCRTPoly>>> ekMap1,
        const std::shared_ptr<std::map<uint32_t, EvalKey<DCRTPoly>>> ekMap2,
        const std::string& keyTag) override {
        return m_real->MultiAddEvalSumKeys(ekMap1, ekMap2, keyTag);
    }
    std::shared_ptr<std::map<uint32_t, EvalKey<DCRTPoly>>> MultiAddEvalAutomorphismKeys(
        const std::shared_ptr<std::map<uint32_t, EvalKey<DCRTPoly>>> ekMap1,
        const std::shared_ptr<std::map<uint32_t, EvalKey<DCRTPoly>>> ekMap2,
        const std::string& keyTag) override {
        return m_real->MultiAddEvalAutomorphismKeys(ekMap1, ekMap2, keyTag);
    }
    PublicKey<DCRTPoly> MultiAddPubKeys(PublicKey<DCRTPoly> pk1,
                                        PublicKey<DCRTPoly> pk2,
                                        const std::string& keyTag) override {
        return m_real->MultiAddPubKeys(pk1, pk2, keyTag);
    }
    EvalKey<DCRTPoly> MultiAddEvalMultKeys(EvalKey<DCRTPoly> ek1,
                                           EvalKey<DCRTPoly> ek2,
                                           const std::string& keyTag) override {
        return m_real->MultiAddEvalMultKeys(ek1, ek2, keyTag);
    }

};

}  // namespace lbcrypto
