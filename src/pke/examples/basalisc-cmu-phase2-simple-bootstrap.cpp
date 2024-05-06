/*! This single file does a sequence of bootstraps and times them.
 * The values are chosen to match the "Bootstrap Metric.docx" description.
 * Compile with OpenFHE -DNATIVE_SIZE=64 -DCMAKE_BUILD_TYPE=Release -DWITH_OPENMP=OFF.
 */
#include <algorithm>
#include <chrono>
#include <iostream>
#include <tuple>

#include "openfhe.h"


using namespace lbcrypto;
using namespace std::chrono;

template<typename ITER>
double median(ITER begin, ITER end) {
    auto n = std::distance(begin, end);
    assert(n > 0);
    if (n % 2 == 1) {
        return *(begin + n / 2);
    } else {
        return 0.5 * (*(begin + n / 2 - 1) + *(begin + n / 2));
    }
}

template<typename ITER>
std::tuple<double,double> median_iqr(ITER begin, ITER end) {
    std::sort(begin, end);
    auto n = std::distance(begin, end);
    return {median(begin, end),  median(end - n / 2, end) - median(begin, begin + n / 2)};
}


int main(int argc, char* argv[]) {
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetSecurityLevel(HEStd_128_classic);
    parameters.SetRingDim(1<<16);
    SecretKeyDist secretKeyDist = UNIFORM_TERNARY;
    parameters.SetSecretKeyDist(secretKeyDist);
    parameters.SetScalingModSize(57);
    parameters.SetFirstModSize(60);
    parameters.SetScalingTechnique(FLEXIBLEAUTO);
    parameters.SetMultiplicativeDepth(20);
    std::cout << "Generate crypto context" << std::endl << std::flush;
    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    cc->Enable(ADVANCEDSHE);
    cc->Enable(FHE);
    int32_t slot_cnt = cc->GetEncodingParams()->GetBatchSize();
    std::cout << "Slot count " << slot_cnt << std::endl;
    std::vector<uint32_t> level_budget = {2, 2};
    std::vector<uint32_t> bsgsDim = {0, 0};
    cc->EvalBootstrapSetup(level_budget, bsgsDim, slot_cnt);
    std::cout << "Create keys" << std::endl << std::flush;
    KeyPair<DCRTPoly> keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    cc->EvalSumKeyGen(keys.secretKey);
    // There aren't rotation keys but this is where you would generate them.
    std::vector<int32_t> rotation_indices;
    cc->EvalRotateKeyGen(keys.secretKey, rotation_indices);

    std::cout << "Bootstrap keygen " << slot_cnt << std::endl;
    cc->EvalBootstrapKeyGen(keys.secretKey, slot_cnt);
    int depth = static_cast<int>(parameters.GetMultiplicativeDepth());
    std::cout << "Full depth " << depth << std::endl;

    std::vector<double> values(slot_cnt, 0);
    std::generate(values.begin(), values.end(), [n = 0, slot_cnt] () mutable {
        n++;
        return double(n) / slot_cnt;
        });

    // Plaintext test_plaintext = cc->MakeCKKSPackedPlaintext(values);
    // auto test_ciphertext = cc->Encrypt(keys.publicKey, test_plaintext);
    // int limb_cnt = test_ciphertext->GetElements()[0].GetAllElements().size();
    // std::cout << "full limb count " << limb_cnt << std::endl;
    int noise_scale_degree{2};
    int drop_cnt = depth - 2;
    Plaintext ptxt = cc->MakeCKKSPackedPlaintext(
        values, noise_scale_degree, drop_cnt, nullptr, slot_cnt);
    ptxt->SetLength(slot_cnt);
    auto ciphertext = cc->Encrypt(keys.publicKey, ptxt);

    // size_t iter_cnt = 1;
    // std::cout << "Bootstrapping " << iter_cnt << " times" << std::endl;
    // std::vector<double> durations(iter_cnt, 0);
    // for (size_t iter_idx = 0; iter_idx < iter_cnt; iter_idx++) {
    high_resolution_clock clock;
    auto start = clock.now();
    [[maybe_unused]] auto bootstrapped = cc->EvalBootstrap(ciphertext);
    double seconds = duration_cast<duration<double>>(clock.now() - start).count();
    std::cout << seconds << std::endl;

    Basalisc.instruction_stats().display();
        // durations[iter_idx] = seconds;
    // }
    // auto [median, iqr] = median_iqr(durations.begin(), durations.end());
    // std::cout << "median " << median << " interquartile range (stdev) " << iqr << std::endl;
    return 0;
}