//==================================================================================
// BSD 2-Clause License
//
// Copyright (c) 2014-2022, NJIT, Duality Technologies Inc. and other contributors
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
  Example of a computation circuit of depth 3.
  BFVrns demo for a homomorphic multiplication of depth 6 and three different approaches for depth-3 multiplications
 */

#define PROFILE

#include <iostream>

#include "openfhe.h"
#include "lattice/stdlatticeparms.h"

using namespace lbcrypto;

int main(int argc, char* argv[]) {
    if(argc<2) {
        std::cerr << "Specify the sweep size (try 6, 7, or 8, maybe) as a command line argument"
                  << std::endl;
        return 1;
    }
    int vector_length = std::stoi(argv[1]);
    int log2_vector_length = 0;
    for(int i = vector_length; i > 1; i = (i+1)/2) ++log2_vector_length;

    std::cout << "Requesting multiplicative depth " << log2_vector_length << " for product of " << vector_length << " ciphertexts." << std::endl;

    if(vector_length < 1) {
        std::cerr << "Sweep size must be positive (saw " << vector_length << ")."
                  << std::endl;
        return 2;
    }

    // benchmarking variables
    TimeVar t;
    double processingTime(0.0);

    CCParams<CryptoContextBFVRNS> parameters;
    parameters.SetPlaintextModulus(536903681);
    parameters.SetMultiplicativeDepth(log2_vector_length);
    parameters.SetMaxRelinSkDeg(3);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cryptoContext = GenCryptoContext(parameters);
    // enable features that you wish to use
    cryptoContext->Enable(PKE);
    cryptoContext->Enable(KEYSWITCH);
    cryptoContext->Enable(LEVELEDSHE);
    cryptoContext->Enable(ADVANCEDSHE);

    std::cout << "\np = " << cryptoContext->GetCryptoParameters()->GetPlaintextModulus() << std::endl;
    std::cout << "n = " << cryptoContext->GetCryptoParameters()->GetElementParams()->GetCyclotomicOrder() / 2
              << std::endl;
    std::cout << "log2 q = "
              << log2(cryptoContext->GetCryptoParameters()->GetElementParams()->GetModulus().ConvertToDouble())
              << std::endl;

    // Initialize Public Key Containers
    KeyPair<DCRTPoly> keyPair;

    ////////////////////////////////////////////////////////////
    // Perform Key Generation Operation
    ////////////////////////////////////////////////////////////

    std::cout << "\nRunning key generation (used for source data)..." << std::endl;

    TIC(t);

    keyPair = cryptoContext->KeyGen();

    processingTime = TOC(t);
    std::cout << "Key generation time: " << processingTime << "ms" << std::endl;

    if (!keyPair.good()) {
        std::cout << "Key generation failed!" << std::endl;
        exit(1);
    }

    std::cout << "Running key generation for homomorphic multiplication "
                 "evaluation keys..."
              << std::endl;

    TIC(t);

    cryptoContext->EvalMultKeysGen(keyPair.secretKey);

    processingTime = TOC(t);
    std::cout << "Key generation time for homomorphic multiplication evaluation keys: " << processingTime << "ms"
              << std::endl;

    // cryptoContext->EvalMultKeyGen(keyPair.secretKey);

    ////////////////////////////////////////////////////////////
    // Encode source data
    ////////////////////////////////////////////////////////////

    std::vector<Plaintext> pts;
    for(int i = 0; i < vector_length; i++) {
        std::vector<int64_t> vectorOfInts = {12*i+1, 12*i+2, 12*i+3, 12*i+4, 12*i+5, 12*i+6, 12*i+7, 12*i+8, 12*i+9, 12*i+10, 12*i+11, 12*i+12};
        pts.push_back(cryptoContext->MakePackedPlaintext(vectorOfInts));
    }

    std::cout << "\nOriginal plaintexts: \n";
    for(auto pt : pts) std::cout << pt << std::endl;

    ////////////////////////////////////////////////////////////
    // Encryption
    ////////////////////////////////////////////////////////////

    std::cout << "\nRunning encryption of all plaintexts... ";

    std::vector<Ciphertext<DCRTPoly>> ciphertexts;

    TIC(t);

    for(auto pt : pts) ciphertexts.push_back(cryptoContext->Encrypt(keyPair.publicKey, pt));

    processingTime = TOC(t);

    std::cout << "Completed\n";

    std::cout << "\nAverage encryption time: " << processingTime / (double)pts.size() << "ms" << std::endl;

    ////////////////////////////////////////////////////////////
    // Homomorphic multiplication of 7 ciphertexts
    ////////////////////////////////////////////////////////////

    std::cout << "\nRunning a binary-tree multiplication of " << ciphertexts.size() << " ciphertexts...";

    TIC(t);

    auto ciphertextMult7 = cryptoContext->EvalMultMany(ciphertexts);

    processingTime = TOC(t);

    std::cout << "Completed\n";

    std::cout << "\nTotal time of multiplying " << ciphertexts.size() << " ciphertexts using EvalMultMany: " << processingTime << "ms"
              << std::endl;

    Plaintext plaintextDecMult7;

    cryptoContext->Decrypt(keyPair.secretKey, ciphertextMult7, &plaintextDecMult7);

    plaintextDecMult7->SetLength(pts[0]->GetLength());

    std::cout << "\nResult of homomorphic multiplications: \n";
    std::cout << plaintextDecMult7 << std::endl;

    return 0;
}
