#ifndef LBCRYPTO_INC_LATTICE_HAL_BASALISC_NATIVEPOLY_H
#define LBCRYPTO_INC_LATTICE_HAL_BASALISC_NATIVEPOLY_H

#include "lattice/hal/poly-interface.h"
#include "lattice/hal/default/ildcrtparams.h"
#include "lattice/hal/default/ilparams.h"

#include "math/distrgen.h"
#include "math/math-hal.h"
#include "math/nbtheory.h"

#include "utils/exception.h"
#include "utils/inttypes.h"

#include <functional>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#define NOTIMPL OPENFHE_THROW(not_implemented_error, "PolyImpl specialization doesn't implement this");

namespace lbcrypto {

template<typename VecType>
class PolyImpl;

template<>
class PolyImpl<NativeVector> : public PolyInterface<PolyImpl<NativeVector>, NativeVector, PolyImpl> {
public:
    using VecType           = NativeVector;
    using Vector            = VecType;
    using Integer           = typename VecType::Integer;
    using Params            = ILParamsImpl<Integer>;
    using PolyNative        = PolyImpl<NativeVector>;
    using PolyType          = PolyImpl<VecType>;
    using PolyLargeType     = PolyImpl<VecType>;
    using PolyInterfaceType = PolyInterface<PolyImpl<VecType>, VecType, PolyImpl>;
    using DggType           = typename PolyInterfaceType::DggType;
    using DugType           = typename PolyInterfaceType::DugType;
    using TugType           = typename PolyInterfaceType::TugType;
    using BugType           = typename PolyInterfaceType::BugType;

    constexpr PolyImpl() = default;

    PolyImpl(const std::shared_ptr<Params>& params, Format format = Format::EVALUATION,
             bool initializeElementToZero = false)
        : m_format{format}, m_params{params} {
            std::cout << "hiii i am specialized";
        if (initializeElementToZero)
            PolyImpl::SetValuesToZero();
    }
    PolyImpl(const std::shared_ptr<ILDCRTParams<Integer>>& params, Format format = Format::EVALUATION,
             bool initializeElementToZero = false)
        : m_format(format), m_params(std::make_shared<Params>(params->GetCyclotomicOrder(), params->GetModulus(), 1)) {
        if (initializeElementToZero)
            this->SetValuesToZero();
    }

    PolyImpl(bool initializeElementToMax, const std::shared_ptr<Params>& params, Format format = Format::EVALUATION)
        : m_format{format}, m_params{params} {
        if (initializeElementToMax)
            PolyImpl::SetValuesToMax();
    }
    PolyImpl(const DggType& dgg, const std::shared_ptr<Params>& params, Format format = Format::EVALUATION);
    PolyImpl(DugType& dug, const std::shared_ptr<Params>& params, Format format = Format::EVALUATION);
    PolyImpl(const BugType& bug, const std::shared_ptr<Params>& params, Format format = Format::EVALUATION);
    PolyImpl(const TugType& tug, const std::shared_ptr<Params>& params, Format format = Format::EVALUATION,
             uint32_t h = 0);

    template <typename T = VecType>
    PolyImpl(const PolyNative& rhs, Format format,
             typename std::enable_if_t<std::is_same_v<T, NativeVector>, bool> = true)
        : m_format{rhs.m_format},
          m_params{rhs.m_params},
          m_values{rhs.m_values ? std::make_unique<VecType>(*rhs.m_values) : nullptr} {
        PolyImpl<VecType>::SetFormat(format);
    }

    template <typename T = VecType>
    PolyImpl(const PolyNative& rhs, Format format,
             typename std::enable_if_t<!std::is_same_v<T, NativeVector>, bool> = true)
        : m_format{rhs.GetFormat()} {
        auto c{rhs.GetParams()->GetCyclotomicOrder()};
        auto m{rhs.GetParams()->GetModulus().ConvertToInt()};
        auto r{rhs.GetParams()->GetRootOfUnity().ConvertToInt()};
        m_params = std::make_shared<PolyImpl::Params>(c, m, r);

        const auto& v{rhs.GetValues()};
        uint32_t vlen{m_params->GetRingDimension()};
        VecType tmp(vlen);
        tmp.SetModulus(m_params->GetModulus());
        for (uint32_t i{0}; i < vlen; ++i)
            tmp[i] = Integer(v[i]);
        m_values = std::make_unique<VecType>(tmp);
        PolyImpl<VecType>::SetFormat(format);
    }

    PolyImpl(const PolyType& p) noexcept
        : m_format{p.m_format},
          m_params{p.m_params},
          m_values{p.m_values ? std::make_unique<VecType>(*p.m_values) : nullptr} {}

    PolyImpl(PolyType&& p) noexcept
        : m_format{p.m_format}, m_params{std::move(p.m_params)}, m_values{std::move(p.m_values)} {}

    PolyType& operator=(const PolyType& rhs) noexcept override;
    PolyType& operator=(PolyType&& rhs) noexcept override {
        m_format = std::move(rhs.m_format);
        m_params = std::move(rhs.m_params);
        m_values = std::move(rhs.m_values);
        return *this;
    }
    PolyType& operator=(const std::vector<int32_t>& rhs) {
        NOTIMPL;
    }

    PolyType& operator=(const std::vector<int64_t>& rhs) {
        NOTIMPL;
    }

    PolyType& operator=(std::initializer_list<uint64_t> rhs) override {
        NOTIMPL;
    }

    PolyType& operator=(std::initializer_list<std::string> rhs) {
        NOTIMPL;
    }

    PolyType& operator=(uint64_t val) {
        NOTIMPL;
    }

    PolyNative DecryptionCRTInterpolate(PlaintextModulus ptm) const override;
    PolyNative ToNativePoly() const final {
        return *this;
    }

    void SetValues(const VecType& values, Format format) override {
        NOTIMPL;
    }

    void SetValues(VecType&& values, Format format) override {
        NOTIMPL;
    }

    void SetValuesToZero() override {
        usint r{m_params->GetRingDimension()};
        m_values = std::make_unique<VecType>(r, m_params->GetModulus());
    }

    void SetValuesToMax() override {
        usint r{m_params->GetRingDimension()};
        auto max{m_params->GetModulus() - Integer(1)};
        m_values = std::make_unique<VecType>(r, m_params->GetModulus(), max);
    }

    inline Format GetFormat() const final {
        return m_format;
    }

    void OverrideFormat(const Format f) final {
        m_format = f;
    }

    inline const std::shared_ptr<Params>& GetParams() const {
        return m_params;
    }

    inline const VecType& GetValues() const final {
        if (m_values == nullptr)
            OPENFHE_THROW("No values in PolyImpl");
        return *m_values;
    }

    inline bool IsEmpty() const final {
        return m_values == nullptr;
    }

    inline Integer& at(usint i) final {
        if (m_values == nullptr)
            OPENFHE_THROW("No values in PolyImpl");
        return m_values->at(i);
    }

    inline const Integer& at(usint i) const final {
        if (m_values == nullptr)
            OPENFHE_THROW("No values in PolyImpl");
        return m_values->at(i);
    }

    inline Integer& operator[](usint i) final {
        return (*m_values)[i];
    }

    inline const Integer& operator[](usint i) const final {
        return (*m_values)[i];
    }

    PolyImpl Plus(const PolyImpl& rhs) const override {
        if (m_params->GetRingDimension() != rhs.m_params->GetRingDimension())
            OPENFHE_THROW("RingDimension missmatch");
        if (m_params->GetModulus() != rhs.m_params->GetModulus())
            OPENFHE_THROW("Modulus missmatch");
        if (m_format != rhs.m_format)
            OPENFHE_THROW("Format missmatch");
        auto tmp(*this);
        tmp.m_values->ModAddNoCheckEq(*rhs.m_values);
        return tmp;
    }
    PolyImpl PlusNoCheck(const PolyImpl& rhs) const {
        auto tmp(*this);
        tmp.m_values->ModAddNoCheckEq(*rhs.m_values);
        return tmp;
    }
    PolyImpl& operator+=(const PolyImpl& element) override {
        NOTIMPL;
    }

    PolyImpl Plus(const Integer& element) const override {
        NOTIMPL;
    }

    PolyImpl& operator+=(const Integer& element) override {
        return *this = this->Plus(element);  // don't change this
    }

    PolyImpl Minus(const PolyImpl& element) const override {
        NOTIMPL;
    }

    PolyImpl& operator-=(const PolyImpl& element) override {
        NOTIMPL;
    }

    PolyImpl Minus(const Integer& element) const override {
        NOTIMPL;
    }

    PolyImpl& operator-=(const Integer& element) override {
        m_values->ModSubEq(element);
        return *this;
    }

    PolyImpl Times(const PolyImpl& rhs) const override {
        if (m_params->GetRingDimension() != rhs.m_params->GetRingDimension())
            OPENFHE_THROW("RingDimension missmatch");
        if (m_params->GetModulus() != rhs.m_params->GetModulus())
            OPENFHE_THROW("Modulus missmatch");
        if (m_format != Format::EVALUATION || rhs.m_format != Format::EVALUATION)
            OPENFHE_THROW("operator* for PolyImpl supported only in Format::EVALUATION");
        auto tmp(*this);
        tmp.m_values->ModMulNoCheckEq(*rhs.m_values);
        return tmp;
    }
    PolyImpl TimesNoCheck(const PolyImpl& rhs) const {
        auto tmp(*this);
        tmp.m_values->ModMulNoCheckEq(*rhs.m_values);
        return tmp;
    }
    PolyImpl& operator*=(const PolyImpl& rhs) override {
        if (m_params->GetRingDimension() != rhs.m_params->GetRingDimension())
            OPENFHE_THROW("RingDimension missmatch");
        if (m_params->GetModulus() != rhs.m_params->GetModulus())
            OPENFHE_THROW("Modulus missmatch");
        if (m_format != Format::EVALUATION || rhs.m_format != Format::EVALUATION)
            OPENFHE_THROW("operator* for PolyImpl supported only in Format::EVALUATION");
        if (m_values) {
            m_values->ModMulNoCheckEq(*rhs.m_values);
            return *this;
        }
        m_values = std::make_unique<VecType>(m_params->GetRingDimension(), m_params->GetModulus());
        return *this;
    }

    PolyImpl Times(const Integer& element) const override {
        NOTIMPL;
    }

    PolyImpl& operator*=(const Integer& element) override {
        m_values->ModMulEq(element);
        return *this;
    }

    PolyImpl Times(NativeInteger::SignedNativeInt element) const override {
        NOTIMPL;
    }

    PolyImpl MultiplyAndRound(const Integer& p, const Integer& q) const override {
        NOTIMPL;
    }

    PolyImpl DivideAndRound(const Integer& q) const override {
        NOTIMPL;
    }

    PolyImpl Negate() const override;
    inline PolyImpl operator-() const override {
        return PolyImpl(m_params, m_format, true) -= *this;
    }

    inline bool operator==(const PolyImpl& rhs) const override {
        return ((m_format == rhs.GetFormat()) && (m_params->GetRootOfUnity() == rhs.GetRootOfUnity()) &&
                (this->GetValues() == rhs.GetValues()));
    }

    void AddILElementOne() override {
        NOTIMPL;
    }

    PolyImpl AutomorphismTransform(uint32_t k) const override {
        NOTIMPL;
    }

    PolyImpl AutomorphismTransform(uint32_t k, const std::vector<uint32_t>& vec) const override {
        NOTIMPL;
    }

    PolyImpl MultiplicativeInverse() const override {
        NOTIMPL;
    }

    PolyImpl ModByTwo() const override {
        NOTIMPL;
    }

    PolyImpl Mod(const Integer& modulus) const override {
        NOTIMPL;
    }

    void SwitchModulus(const Integer& modulus, const Integer& rootOfUnity, const Integer& modulusArb,
                       const Integer& rootOfUnityArb) override {
        NOTIMPL;
    }

    void SwitchFormat() override {
        NOTIMPL;
    }

    void MakeSparse(uint32_t wFactor) override {
        NOTIMPL;
    }

    bool InverseExists() const override {
        NOTIMPL;
    }

    double Norm() const override {
        NOTIMPL;
    }
    std::vector<PolyImpl> BaseDecompose(usint baseBits, bool evalModeAnswer) const override {
        NOTIMPL;
    }
    std::vector<PolyImpl> PowersOfBase(usint baseBits) const override {
        NOTIMPL;
    }

    template <class Archive>
    void save(Archive& ar, std::uint32_t const version) const {
        ar(::cereal::make_nvp("v", m_values));
        ar(::cereal::make_nvp("f", m_format));
        ar(::cereal::make_nvp("p", m_params));
    }

    template <class Archive>
    void load(Archive& ar, std::uint32_t const version) {
        if (version > SerializedVersion()) {
            OPENFHE_THROW("serialized object version " + std::to_string(version) +
                          " is from a later version of the library");
        }
        ar(::cereal::make_nvp("v", m_values));
        ar(::cereal::make_nvp("f", m_format));
        ar(::cereal::make_nvp("p", m_params));
    }

    static const std::string GetElementName() {
        return "PolyImpl";
    }

    std::string SerializedObjectName() const override {
        return "Poly";
    }

    static uint32_t SerializedVersion() {
        return 1;
    }

protected:
    Format m_format{Format::EVALUATION};
    std::shared_ptr<Params> m_params{nullptr};
    std::unique_ptr<VecType> m_values{nullptr};
    void ArbitrarySwitchFormat() {
        NOTIMPL;
    }
};

}

#endif