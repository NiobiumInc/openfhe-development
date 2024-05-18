|                         |                            |                | BGV |               | CKKS |                | BFV |  |  |  |
|-------------------------|----------------------------|----------------|------|---------------|------|----------------|------|---------------|----------------|-------------|
| Parameter name          | Parameter set function     | Parameter type | Used | Default value | Used | Default value  | Used | Default value | Comments       | Description |
| scheme                  | NA                         | SCHEME         | Y    | BGVRNS_SCHEME | Y    | CKKSRNS_SCHEME | Y    | BFVRNS_SCHEME | Set internally |             |
| ptModulus               | SetPlaintextModulus        | PlaintextModulus | Y | 0 | N | NA | Y | 0 | CKKS: default value - 0, not settable by users; BFV/BGV: throw an exception if the ptModulus = 0 asking the user to set it  | Plaintext modulus |
| digitSize               | SetDigitSize               | uint32_t | Y | 0 | Y | 0 | Y | 0 |  |  |
| standardDeviation       | SetStandardDeviation       | float | Y | 3.19 | Y | 3.19 | Y | 3.19 |  | Error distribution parameter (only for advacned users) |
| secretKeyDist           | SetSecretKeyDist           | SecretKeyDist | Y | UNIFORM_TERNARY | Y | UNIFORM_TERNARY | Y | UNIFORM_TERNARY |  | Selects the distribution for secret keys (ternary uniform or sparse) |
| maxRelinSkDeg           | SetMaxRelinSkDeg           | int | Y | 2 | Y | 2 | Y | 2 |  | Keep |
| ksTech                  | SetKeySwitchTechnique      | KeySwitchTechnique | Y | HYBRID | Y | HYBRID | Y | BV |  |  |
| scalTech                | SetScalingTechnique        | ScalingTechnique | Y | FLEXIBLEAUTOEXT | Y | FIXEDAUTO (128-bit)/ FLEXIBLEAUTOEXT | N | NA | BFV: default value - NORESCALE (effectively the equivalent for NA or just an option that does nothing); CKKS/BGV: may not use NORESCALE |  |
| firstModSize            | SetFirstModSize            | uint32_t | Y | 0 | Y | 89 (128-bit) / 60 | N | NA | firstModSize for BGV is allowed only for FIXEDMANUAL |  |
| scalingModSize          | SetScalingModSize          | uint32_t | N | NA | Y | 78 (128-bit) / 50 | Y | 60 (128-bit) / 57 | BGV: scalingModSize is computed internally |  |
| batchSize               | SetBatchSize               | uint32_t | Y | 0 | Y | 0 | Y | 0 |  |  |
| numLargeDigits          | SetNumLargeDigits          | uint32_t | Y | 0 | Y | 0 | Y | 0 |  | Number of digits in hybrid key switching |
| multiplicativeDepth     | SetMultiplicativeDepth     | uint32_t | Y | 1 | Y | 1 | Y | 1 |  |  |
| securityLevel           | SetSecurityLevel           | SecurityLevel | Y | HEStd_128_classic | Y | HEStd_128_classic | Y | HEStd_128_classic |  |  |
| ringDim                 | SetRingDim                 | uint32_t | Y | 0 | Y | 0 | Y | 0 |  |  |
| evalAddCount            | SetEvalAddCount            | uint32_t | Y | 5 | N | NA | Y | 0 |  |  |
| keySwitchCount          | SetKeySwitchCount          | uint32_t | Y | 3 | N | NA | Y | 0 |  |  |
| encryptionTechnique     | SetEncryptionTechnique     | EncryptionTechnique | N | NA | N | NA | Y | STANDARD | BGV/CKKS: use STANDARD internally |  |
| multiplicationTechnique | SetMultiplicationTechnique | MultiplicationTechnique | N | NA | N | NA | Y | HPSPOVERQLEVELED |  |  |
| multiHopModSize         | SetMultiHopModSize         | uint32_t | Y | 0 | N | NA | N | NA | Used only when MultipartyMode = NOISE_FLOODING_MULTIPARTY |  |
| PREMode                 | SetPREMode                 | ProxyReEncryptionMode | Y | INDCPA | Y | INDCPA | Y | INDCPA | NOISE_FLOODING_HRA supported only in BGV; Remove DIVIDE_AND_ROUND_HRA |  |
| multipartyMode          | SetMultipartyMode          | MultipartyMode | Y | FIXED_NOISE_MULTIPARTY | N | NA | Y | FIXED_NOISE_MULTIPARTY | CKKS: FIXED_NOISE_MULTIPARTY only |  |
| executionMode           | SetExecutionMode           | ExecutionMode | N | NA | Y | EXEC_EVALUATION | N | NA | BGV/BFV: EXEC_EVALUATION only |  |
| decryptionNoiseMode     | SetDecryptionNoiseMode     | DecryptionNoiseMode | N | NA | Y | FIXED_NOISE_DECRYPT | N | NA |  |  |
| noiseEstimate           | SetNoiseEstimate           | double | N | NA | Y | 0 | N | NA |  |  |
| desiredPrecision        | SetDesiredPrecision        | double | N | NA | Y | 25 | N | NA |  |  |
| statisticalSecurity     | SetStatisticalSecurity     | uint32_t | Y | 30 | Y | 30 | N | NA |  |  |
| numAdversarialQueries   | SetNumAdversarialQueries   | uint32_t | Y | 1 | Y | 1 | N | NA |  |  |
| thresholdNumOfParties   | SetThresholdNumOfParties   | uint32_t | Y | 1 | N | NA | Y | 1 |  |  |
| interactiveBootCompressionLevel | SetInteractiveBootCompressionLevel | COMPRESSION_LEVEL | N | NA | Y | SLACK | N | NA |  |  |
