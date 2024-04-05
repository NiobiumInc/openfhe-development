//==================================================================================
// BSD 2-Clause License
//
// Copyright (c) 2014-2023, NJIT, Duality Technologies Inc. and other contributors
//
// All rights reserved.
//
// Author TPOC: contact@openfhe.org
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//==================================================================================

/*
  implementation of the integer lattice
 */


#include "lattice/hal/basalisc/poly.h"

#include "utils/debug.h"
#include "utils/exception.h"
#include "utils/inttypes.h"

#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace lbcrypto {

BasPoly<NativeVector>::BasPoly(const DggType& dgg, const std::shared_ptr<BasPoly::Params>& params, Format format)
    : m_format{Format::COEFFICIENT},
      m_params{params},
      m_values{std::make_unique<NativeVector>(dgg.GenerateVector(params->GetRingDimension(), params->GetModulus()))} {
    BasPoly<NativeVector>::SetFormat(format);
}

BasPoly<NativeVector>::BasPoly(DugType& dug, const std::shared_ptr<BasPoly::Params>& params, Format format)
    : m_format{format},
      m_params{params},
      m_values{std::make_unique<NativeVector>(dug.GenerateVector(params->GetRingDimension(), params->GetModulus()))} {}

BasPoly<NativeVector>::BasPoly(const BugType& bug, const std::shared_ptr<BasPoly::Params>& params, Format format)
    : m_format{Format::COEFFICIENT},
      m_params{params},
      m_values{std::make_unique<NativeVector>(bug.GenerateVector(params->GetRingDimension(), params->GetModulus()))} {
    BasPoly<NativeVector>::SetFormat(format);
}

BasPoly<NativeVector>::BasPoly(const TugType& tug, const std::shared_ptr<BasPoly::Params>& params, Format format,
                            uint32_t h)
    : m_format{Format::COEFFICIENT},
      m_params{params},
      m_values{std::make_unique<NativeVector>(tug.GenerateVector(params->GetRingDimension(), params->GetModulus(), h))} {
    BasPoly<NativeVector>::SetFormat(format);
}

BasPoly<NativeVector>& BasPoly<NativeVector>::operator=(const BasPoly& rhs) noexcept {
    m_format = rhs.m_format;
    m_params = rhs.m_params;
    if (!rhs.m_values) {
        m_values = nullptr;
        return *this;
    }
    if (m_values) {
        *m_values = *rhs.m_values;
        return *this;
    }
    m_values = std::make_unique<NativeVector>(*rhs.m_values);
    return *this;
}

// assumes that elements in rhs less than modulus?
BasPoly<NativeVector>& BasPoly<NativeVector>::operator=(std::initializer_list<uint64_t> rhs) {
    static const Integer ZERO(0);
    const size_t llen = rhs.size();
    const size_t vlen = m_params->GetRingDimension();
    if (!m_values) {
        NativeVector temp(vlen);
        temp.SetModulus(m_params->GetModulus());
        BasPoly<NativeVector>::SetValues(std::move(temp), m_format);
    }
    for (size_t j = 0; j < vlen; ++j)
        (*m_values)[j] = (j < llen) ? *(rhs.begin() + j) : ZERO;
    return *this;
}

// TODO: template with enable_if int64_t/int32_t
BasPoly<NativeVector>& BasPoly<NativeVector>::operator=(const std::vector<int64_t>& rhs) {
    static const Integer ZERO(0);
    m_format = Format::COEFFICIENT;
    const size_t llen{rhs.size()};
    const size_t vlen{m_params->GetRingDimension()};
    const auto& m = m_params->GetModulus();
    if (!m_values) {
        NativeVector tmp(vlen);
        tmp.SetModulus(m);
        BasPoly<NativeVector>::SetValues(std::move(tmp), m_format);
    }
    for (size_t j = 0; j < vlen; ++j) {
        if (j < llen)
            (*m_values)[j] =
                (rhs[j] < 0) ? m - Integer(static_cast<uint64_t>(-rhs[j])) : Integer(static_cast<uint64_t>(rhs[j]));
        else
            (*m_values)[j] = ZERO;
    }
    return *this;
}

BasPoly<NativeVector>& BasPoly<NativeVector>::operator=(const std::vector<int32_t>& rhs) {
    static const Integer ZERO(0);
    m_format = Format::COEFFICIENT;
    const size_t llen{rhs.size()};
    const size_t vlen{m_params->GetRingDimension()};
    const auto& m = m_params->GetModulus();
    if (!m_values) {
        NativeVector tmp(vlen);
        tmp.SetModulus(m);
        BasPoly<NativeVector>::SetValues(std::move(tmp), m_format);
    }
    for (size_t j = 0; j < vlen; ++j) {
        if (j < llen)
            (*m_values)[j] =
                (rhs[j] < 0) ? m - Integer(static_cast<uint64_t>(-rhs[j])) : Integer(static_cast<uint64_t>(rhs[j]));
        else
            (*m_values)[j] = ZERO;
    }
    return *this;
}

BasPoly<NativeVector>& BasPoly<NativeVector>::operator=(std::initializer_list<std::string> rhs) {
    const size_t vlen = m_params->GetRingDimension();
    if (!m_values) {
        NativeVector temp(vlen);
        temp.SetModulus(m_params->GetModulus());
        BasPoly<NativeVector>::SetValues(std::move(temp), m_format);
    }
    *m_values = rhs;
    return *this;
}

BasPoly<NativeVector>& BasPoly<NativeVector>::operator=(uint64_t val) {
    m_format = Format::EVALUATION;
    if (!m_values) {
        auto d{m_params->GetRingDimension()};
        const auto& m{m_params->GetModulus()};
        m_values = std::make_unique<NativeVector>(d, m);
    }
    size_t vlen{m_values->GetLength()};
    Integer ival{val};
    for (size_t i = 0; i < vlen; ++i)
        (*m_values)[i] = ival;
    return *this;
}

void BasPoly<NativeVector>::SetValues(const NativeVector& values, Format format) {
    if (m_params->GetRootOfUnity() == Integer(0))
        OPENFHE_THROW("Polynomial has a 0 root of unity");
    if (m_params->GetRingDimension() != values.GetLength() || m_params->GetModulus() != values.GetModulus())
        OPENFHE_THROW("Parameter mismatch on SetValues for Polynomial");
    m_format = format;
    m_values = std::make_unique<NativeVector>(values);
}

void BasPoly<NativeVector>::SetValues(NativeVector&& values, Format format) {
    if (m_params->GetRootOfUnity() == Integer(0))
        OPENFHE_THROW("Polynomial has a 0 root of unity");
    if (m_params->GetRingDimension() != values.GetLength() || m_params->GetModulus() != values.GetModulus())
        OPENFHE_THROW("Parameter mismatch on SetValues for Polynomial");
    m_format = format;
    m_values = std::make_unique<NativeVector>(std::move(values));
}

BasPoly<NativeVector> BasPoly<NativeVector>::Plus(const typename NativeVector::Integer& element) const {
    BasPoly<NativeVector> tmp(m_params, m_format);
    if (m_format == Format::COEFFICIENT)
        tmp.SetValues((*m_values).ModAddAtIndex(0, element), m_format);
    else
        tmp.SetValues((*m_values).ModAdd(element), m_format);
    return tmp;
}

BasPoly<NativeVector> BasPoly<NativeVector>::Minus(const typename NativeVector::Integer& element) const {
    BasPoly<NativeVector> tmp(m_params, m_format);
    tmp.SetValues((*m_values).ModSub(element), m_format);
    return tmp;
}

BasPoly<NativeVector> BasPoly<NativeVector>::Times(const typename NativeVector::Integer& element) const {
    BasPoly<NativeVector> tmp(m_params, m_format);
    tmp.SetValues((*m_values).ModMul(element), m_format);
    return tmp;
}

BasPoly<NativeVector> BasPoly<NativeVector>::Times(NativeInteger::SignedNativeInt element) const {
    BasPoly<NativeVector> tmp(m_params, m_format);
    Integer q{m_params->GetModulus()};
    if (element < 0) {
        Integer elementReduced{NativeInteger::Integer(-element)};
        if (elementReduced > q)
            elementReduced.ModEq(q);
        tmp.SetValues((*m_values).ModMul(q - elementReduced), m_format);
    }
    else {
        Integer elementReduced{NativeInteger::Integer(element)};
        if (elementReduced > q)
            elementReduced.ModEq(q);
        tmp.SetValues((*m_values).ModMul(elementReduced), m_format);
    }
    return tmp;
}

BasPoly<NativeVector> BasPoly<NativeVector>::Minus(const BasPoly& rhs) const {
    BasPoly<NativeVector> tmp(m_params, m_format);
    tmp.SetValues((*m_values).ModSub(*rhs.m_values), m_format);
    return tmp;
}

BasPoly<NativeVector> BasPoly<NativeVector>::MultiplyAndRound(const typename NativeVector::Integer& p,
                                                      const typename NativeVector::Integer& q) const {
    BasPoly<NativeVector> tmp(m_params, m_format);
    tmp.SetValues((*m_values).MultiplyAndRound(p, q), m_format);
    return tmp;
}

BasPoly<NativeVector> BasPoly<NativeVector>::DivideAndRound(const typename NativeVector::Integer& q) const {
    BasPoly<NativeVector> tmp(m_params, m_format);
    tmp.SetValues((*m_values).DivideAndRound(q), m_format);
    return tmp;
}

// TODO: this will return vec of 0s for BigIntegers
BasPoly<NativeVector> BasPoly<NativeVector>::Negate() const {
    //  UnitTestBFVrnsCRTOperations.cpp line 316 throws with this uncommented
    //    if (m_format != Format::EVALUATION)
    //        OPENFHE_THROW("Negate for BasPoly is supported only in Format::EVALUATION format.\n");
    return BasPoly<NativeVector>(m_params, m_format, true) -= *this;
}

BasPoly<NativeVector>& BasPoly<NativeVector>::operator+=(const BasPoly& element) {
    if (!m_values)
        m_values = std::make_unique<NativeVector>(m_params->GetRingDimension(), m_params->GetModulus());
    m_values->ModAddEq(*element.m_values);
    return *this;
}

BasPoly<NativeVector>& BasPoly<NativeVector>::operator-=(const BasPoly& element) {
    if (!m_values)
        m_values = std::make_unique<NativeVector>(m_params->GetRingDimension(), m_params->GetModulus());
    m_values->ModSubEq(*element.m_values);
    return *this;
}

void BasPoly<NativeVector>::AddILElementOne() {
    static const Integer ONE(1);
    usint vlen{m_params->GetRingDimension()};
    const auto& m{m_params->GetModulus()};
    for (usint i = 0; i < vlen; ++i)
        (*m_values)[i].ModAddFastEq(ONE, m);
}

BasPoly<NativeVector> BasPoly<NativeVector>::AutomorphismTransform(uint32_t k) const {
    uint32_t n{m_params->GetRingDimension()};
    uint32_t m{m_params->GetCyclotomicOrder()};
    bool bp{n == (m >> 1)};
    bool bf{m_format == Format::EVALUATION};

    // if (!bf && !bp)
    if (!bp)
        OPENFHE_THROW("Automorphism Poly Format not EVALUATION or not power-of-two");
    /*
    // TODO: is this branch ever called?

    BasPoly<NativeVector> result(m_params, m_format, true);
    if (bf && !bp) {
        // TODO: Add a test based on the inverse totient hash table?

        // All automorphism operations are performed for k coprime to m
        auto totientList = GetTotientList(m);

        // This step can be eliminated by using a hash table that looks up the
        // ring index (between 0 and n - 1) based on the totient index (between 0 and m - 1)
        NativeVector expanded(m, m_params->GetModulus());
        for (uint32_t i = 0; i < n; ++i)
            expanded[totientList[i]] = (*m_values)[i];

        for (uint32_t i = 0; i < n; ++i) {
            // determines which power of primitive root unity we should switch to
            (*result.m_values)[i] = expanded[totientList[i] * k % m];
        }
        return result;
    }
*/
    if (k % 2 == 0)
        OPENFHE_THROW("Automorphism index not odd\n");

    BasPoly<NativeVector> result(m_params, m_format, true);
    uint32_t logm{lbcrypto::GetMSB(m) - 1};
    uint32_t logn{logm - 1};
    uint32_t mask{(uint32_t(1) << logn) - 1};

    if (bf) {
        for (uint32_t j{0}, jk{k}; j < n; ++j, jk += (2 * k)) {
            auto&& jrev{lbcrypto::ReverseBits(j, logn)};
            auto&& idxrev{lbcrypto::ReverseBits((jk >> 1) & mask, logn)};
            (*result.m_values)[jrev] = (*m_values)[idxrev];
        }
        return result;
    }

    auto q{m_params->GetModulus()};
    for (uint32_t j{0}, jk{0}; j < n; ++j, jk += k)
        (*result.m_values)[jk & mask] = ((jk >> logn) & 0x1) ? q - (*m_values)[j] : (*m_values)[j];
    return result;
}

BasPoly<NativeVector> BasPoly<NativeVector>::AutomorphismTransform(uint32_t k, const std::vector<uint32_t>& precomp) const {
    if ((m_format != Format::EVALUATION) || (m_params->GetRingDimension() != (m_params->GetCyclotomicOrder() >> 1)))
        OPENFHE_THROW("Automorphism Poly Format not EVALUATION or not power-of-two");
    if (k % 2 == 0)
        OPENFHE_THROW("Automorphism index not odd\n");
    BasPoly<NativeVector> tmp(m_params, m_format, true);
    uint32_t n = m_params->GetRingDimension();
    for (uint32_t j = 0; j < n; ++j)
        (*tmp.m_values)[j] = (*m_values)[precomp[j]];
    return tmp;
}

BasPoly<NativeVector> BasPoly<NativeVector>::MultiplicativeInverse() const {
    BasPoly<NativeVector> tmp(m_params, m_format);
    tmp.SetValues((*m_values).ModInverse(), m_format);
    return tmp;
}

BasPoly<NativeVector> BasPoly<NativeVector>::ModByTwo() const {
    BasPoly<NativeVector> tmp(m_params, m_format);
    tmp.SetValues((*m_values).ModByTwo(), m_format);
    return tmp;
}

BasPoly<NativeVector> BasPoly<NativeVector>::Mod(const Integer& modulus) const {
    BasPoly<NativeVector> tmp(m_params, m_format);
    tmp.SetValues((*m_values).Mod(modulus), m_format);
    return tmp;
}

void BasPoly<NativeVector>::SwitchModulus(const Integer& modulus, const Integer& rootOfUnity, const Integer& modulusArb,
                                      const Integer& rootOfUnityArb) {
    if (m_values != nullptr) {
        m_values->SwitchModulus(modulus);
        auto c{m_params->GetCyclotomicOrder()};
        m_params = std::make_shared<BasPoly::Params>(c, modulus, rootOfUnity, modulusArb, rootOfUnityArb);
    }
}

void BasPoly<NativeVector>::SwitchFormat() {
    const auto& co{m_params->GetCyclotomicOrder()};
    const auto& rd{m_params->GetRingDimension()};
    const auto& ru{m_params->GetRootOfUnity()};

    if (rd != (co >> 1)) {
        BasPoly<NativeVector>::ArbitrarySwitchFormat();
        return;
    }

    if (!m_values)
        OPENFHE_THROW("Poly switch format to empty values");

    if (m_format != Format::COEFFICIENT) {
        m_format = Format::COEFFICIENT;
        ChineseRemainderTransformFTT<NativeVector>().InverseTransformFromBitReverseInPlace(ru, co, &(*m_values));
        return;
    }
    m_format = Format::EVALUATION;
    ChineseRemainderTransformFTT<NativeVector>().ForwardTransformToBitReverseInPlace(ru, co, &(*m_values));
}

void BasPoly<NativeVector>::ArbitrarySwitchFormat() {
    if (m_values == nullptr)
        OPENFHE_THROW("Poly switch format to empty values");
    const auto& lr = m_params->GetRootOfUnity();
    const auto& bm = m_params->GetBigModulus();
    const auto& br = m_params->GetBigRootOfUnity();
    const auto& co = m_params->GetCyclotomicOrder();
    if (m_format == Format::COEFFICIENT) {
        m_format = Format::EVALUATION;
        auto&& v = ChineseRemainderTransformArb<NativeVector>().ForwardTransform(*m_values, lr, bm, br, co);
        m_values = std::make_unique<NativeVector>(v);
    }
    else {
        m_format = Format::COEFFICIENT;
        auto&& v = ChineseRemainderTransformArb<NativeVector>().InverseTransform(*m_values, lr, bm, br, co);
        m_values = std::make_unique<NativeVector>(v);
    }
}

#if 0
template <>
std::ostream& operator<<(std::ostream& os, const BasPoly<NativeVector>& p) {
    if (p.m_values != nullptr) {
        os << *(p.m_values);
        os << " mod:" << (p.m_values)->GetModulus() << std::endl;
    }
    if (p.m_params.get() != nullptr)
        os << " rootOfUnity: " << p.GetRootOfUnity() << std::endl;
    else
        os << " something's odd: null m_params?!" << std::endl;
    os << std::endl;
    return os;
}
#endif

void BasPoly<NativeVector>::MakeSparse(uint32_t wFactor) {
    static const Integer ZERO(0);
    if (m_values != nullptr) {
        uint32_t vlen{m_params->GetRingDimension()};
        for (uint32_t i = 0; i < vlen; ++i) {
            if (i % wFactor != 0)
                (*m_values)[i] = ZERO;
        }
    }
}

bool BasPoly<NativeVector>::InverseExists() const {
    static const Integer ZERO(0);
    usint vlen{m_params->GetRingDimension()};
    for (usint i = 0; i < vlen; ++i) {
        if ((*m_values)[i] == ZERO)
            return false;
    }
    return true;
}

double BasPoly<NativeVector>::Norm() const {
    usint vlen{m_params->GetRingDimension()};
    const auto& q{m_params->GetModulus()};
    const auto& half{q >> 1};
    Integer maxVal{}, minVal{q};
    for (usint i = 0; i < vlen; i++) {
        auto& val = (*m_values)[i];
        if (val > half)
            minVal = val < minVal ? val : minVal;
        else
            maxVal = val > maxVal ? val : maxVal;
    }
    minVal = q - minVal;
    return (minVal > maxVal ? minVal : maxVal).ConvertToDouble();
}

// Write vector x(current value of the BasPoly object) as \sum\limits{ i = 0
// }^{\lfloor{ \log q / base } \rfloor} {(base^i u_i)} and return the vector of{
// u_0, u_1,...,u_{ \lfloor{ \log q / base } \rfloor } } \in R_base^{ \lceil{
// \log q / base } \rceil }; used as a subroutine in the relinearization
// procedure baseBits is the number of bits in the base, i.e., base = 2^baseBits

// TODO: optimize this
std::vector<BasPoly<NativeVector>> BasPoly<NativeVector>::BaseDecompose(usint baseBits, bool evalModeAnswer) const {
    usint nBits = m_params->GetModulus().GetLengthForBase(2);

    usint nWindows = nBits / baseBits;
    if (nBits % baseBits > 0)
        nWindows++;

    BasPoly<NativeVector> xDigit(m_params);

    std::vector<BasPoly<NativeVector>> result;
    result.reserve(nWindows);

    BasPoly<NativeVector> x(*this);
    x.SetFormat(Format::COEFFICIENT);

    // TP: x is same for BACKEND 2 and 6
    for (usint i = 0; i < nWindows; ++i) {
        xDigit.SetValues(x.GetValues().GetDigitAtIndexForBase(i + 1, 1 << baseBits), x.GetFormat());

        // TP: xDigit is all zeros for BACKEND=6, but not for BACKEND-2
        // *********************************************************
        if (evalModeAnswer)
            xDigit.SwitchFormat();
        result.push_back(xDigit);
    }
    return result;
}

// Generate a vector of BasPoly's as {x, base*x, base^2*x, ..., base^{\lfloor
// {\log q/base} \rfloor}*x, where x is the current BasPoly object; used as a
// subroutine in the relinearization procedure to get powers of a certain "base"
// for the secret key element baseBits is the number of bits in the base, i.e.,
// base = 2^baseBits

std::vector<BasPoly<NativeVector>> BasPoly<NativeVector>::PowersOfBase(usint baseBits) const {
    static const Integer TWO(2);
    const auto& m{m_params->GetModulus()};
    usint nBits{m.GetLengthForBase(2)};
    usint nWindows{nBits / baseBits};
    if (nBits % baseBits > 0)
        ++nWindows;
    std::vector<BasPoly<NativeVector>> result(nWindows);
    Integer shift{0}, bbits{baseBits};
    for (usint i = 0; i < nWindows; ++i, shift += bbits)
        result[i] = (*this) * TWO.ModExp(shift, m);
    return result;
}

typename BasPoly<NativeVector>::PolyNative BasPoly<NativeVector>::DecryptionCRTInterpolate(PlaintextModulus ptm) const {
    const BasPoly<NativeVector> smaller(BasPoly<NativeVector>::Mod(ptm));
    usint vlen{m_params->GetRingDimension()};
    auto c{m_params->GetCyclotomicOrder()};
    auto params{std::make_shared<ILNativeParams>(c, NativeInteger(ptm), 1)};
    typename BasPoly<NativeVector>::PolyNative tmp(params, m_format, true);
    for (usint i = 0; i < vlen; ++i)
        tmp[i] = NativeInteger((*smaller.m_values)[i]);
    return tmp;
}

inline BasPoly<NativeVector> BasPoly<NativeVector>::ToNativePoly() const {
  return *this;
}

}  // namespace lbcrypto

