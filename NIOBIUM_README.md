# Niobium-Specific OpenFHE Modifications

This document describes the Niobium-specific changes made to the OpenFHE library fork.

## Table of Contents

- [CKKS Decode Noise Control](#ckks-decode-noise-control)
  - [Location](#location)
  - [Overview](#overview)
  - [Background: Why Noise is Added During Decode](#background-why-noise-is-added-during-decode)
  - [How to Use](#how-to-use)
  - [Use Cases](#use-cases)
  - [Security Warning](#security-warning)
- [CKKS Decryption Error Override](#ckks-decryption-error-override)
  - [Location](#location-1)
  - [Overview](#overview-1)
  - [Background: Approximation Error in CKKS](#background-approximation-error-in-ckks)
  - [How to Use](#how-to-use-1)
  - [Use Cases](#use-cases-1)
  - [Important Warnings](#important-warnings)

---


## Summary of Niobium Environment Variables

| Variable | Purpose | Security Impact | When to Use |
|----------|---------|-----------------|-------------|
| `NB_NO_DECODE_NOISE` | Disable security noise during decode | **Reduces security** - enables key recovery attacks | Testing/debugging only when deterministic results needed |
| `NB_ALWAYS_DECRYPT` | Bypass approximation error checks | **Allows incorrect results** - returns corrupted/meaningless data | Debugging only when investigating parameter failures |

**Neither variable should EVER be used in production.**

## CKKS Decode Noise Control

### Location
[src/pke/lib/encoding/ckkspackedencoding.cpp:476-485](src/pke/lib/encoding/ckkspackedencoding.cpp#L476-L485)

### Overview
A modification has been added to the CKKS decoding process to allow disabling the security-enhancing Gaussian noise addition during decryption. This is controlled via an environment variable and is intended for testing and deterministic behavior validation only.

### Background: Why Noise is Added During Decode

By default, OpenFHE's CKKS implementation adds Gaussian noise during decryption as a security measure to prevent **key recovery attacks**. Here's why:

1. **Attack Vector**: An adversary with access to decryption queries could potentially recover the secret key by analyzing the noise patterns in decrypted results
2. **Countermeasure**: Adding extra Gaussian noise during decode "drowns out" the underlying noise signature, making it statistically harder to extract information about the secret key
3. **Noise Standard Deviation**: The added noise has standard deviation of `sqrt(M+1)*stddev`, where:
   - `stddev` is estimated from the imaginary component of the decoded result
   - `M` is an extra safety factor (default = 1) that increases the number of decryption queries needed to average out the added noise
   - By default, this requires at least 128 decryption queries to recover useful information

### How to Use

#### Enable Deterministic Decoding (Testing Only)
To disable the security noise and get deterministic decoding results:

```bash
# Set the environment variable before running your program
export NB_NO_DECODE_NOISE=1

# Run your FHE program
./your_fhe_program

# Or set it inline for a single execution
NB_NO_DECODE_NOISE=1 ./your_fhe_program
```

When enabled, you'll see a warning message:
```
[CKKS_DECODE] WARNING: NB_NO_DECODE_NOISE is set - disabling security noise!
```

#### Default Behavior (Production)
By default, or when the variable is unset, the security noise is added:

```bash
# Unset the variable to restore default behavior
unset NB_NO_DECODE_NOISE

# Or simply don't set it
./your_fhe_program
```

### Use Cases

**When to Enable (NB_NO_DECODE_NOISE=1):**
- **Deterministic Testing**: When you need reproducible results across multiple runs for unit tests or validation
- **Debugging**: When comparing exact numerical outputs between different implementations
- **Development**: When testing the record/replay system and need exact value matching
- **Benchmarking**: When measuring pure computation performance without random noise effects

**When to Keep Disabled (Default):**
- **Production Systems**: Any deployment where security matters
- **Multi-Party Computation**: When decryption queries might be accessible to adversaries
- **Cloud FHE Services**: Where users submit ciphertexts and receive decrypted results

### Security Warning

**IMPORTANT**: This modification reduces security and should NEVER be used in production environments or anywhere an adversary might have access to decryption results. The added noise is a critical defense against key recovery attacks.

The implementation includes:
- A warning message printed to stderr when noise is disabled
- A comment indicating this is "for testing only"
- The environment variable name clearly indicates this is Niobium-specific (`NB_`)

---

## CKKS Decryption Error Override

### Location
[src/pke/lib/encoding/ckkspackedencoding.cpp:459-493](src/pke/lib/encoding/ckkspackedencoding.cpp#L459-L493)

### Overview
A modification has been added to allow decryption to proceed even when the approximation error is too high (less than 5 bits of precision). This is controlled via the `NB_ALWAYS_DECRYPT` environment variable and is intended **strictly for testing and debugging purposes** where you need to see results despite parameter mismatches or computational errors.

### Background: Approximation Error in CKKS

CKKS is an approximate encryption scheme where decrypted values have inherent noise. OpenFHE validates that decryption results maintain sufficient precision:

1. **Precision Check**: The library estimates the log standard deviation of the approximation error during decoding
2. **Threshold**: If the approximation error exceeds the threshold (logstd > p - 5.0, where p is the plaintext modulus bits), decryption fails
3. **Failure Reason**: High approximation error typically indicates:
   - Incorrect FHE parameters (depth, scale, modulus chain)
   - Too many operations without bootstrapping
   - Accumulated noise beyond safe limits
   - Computational errors that corrupted the ciphertext

By default, OpenFHE throws an exception when precision is too low to prevent returning meaningless or corrupted results.

### How to Use

#### Force Decryption Despite Errors (Testing/Debugging Only)
To bypass the approximation error check and attempt decryption anyway:

```bash
# Set the environment variable before running your program
export NB_ALWAYS_DECRYPT=1

# Run your FHE program
./your_fhe_program

# Or set it inline for a single execution
NB_ALWAYS_DECRYPT=1 ./your_fhe_program
```

When enabled, you'll see two types of warnings:

**Initial Warning (once at program start):**
```
=========================================================================
  WARNING: NB_ALWAYS_DECRYPT is set!
  Decryption will proceed even with HIGH APPROXIMATION ERRORS.
  THIS IS FOR TESTING/DEBUGGING ONLY - RESULTS MAY BE INCORRECT!
=========================================================================
```

**Per-Decryption Warning (each time error threshold is exceeded):**
```
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  CRITICAL WARNING: APPROXIMATION ERROR IS TOO HIGH!
  Log standard deviation: 58.3 (threshold: 55.0)
  THE DECRYPTION IS INCORRECT AND RESULTS ARE UNRELIABLE!
  Proceeding anyway because NB_ALWAYS_DECRYPT is set.
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
```

#### Default Behavior (Production)
By default, or when the variable is unset, decryption will throw an exception on high approximation errors:

```bash
# Unset the variable to restore default behavior
unset NB_ALWAYS_DECRYPT

# Or simply don't set it
./your_fhe_program
```

### Use Cases

**When to Enable (NB_ALWAYS_DECRYPT=1):**
- **Debugging Parameter Issues**: When you want to see what values come out despite bad parameters
- **Error Investigation**: When analyzing why a computation went wrong and need to inspect partial results
- **Development Testing**: When iteratively tuning parameters and want to see degradation patterns
- **Comparative Analysis**: When comparing results across different parameter sets, even failing ones
- **Fault Tolerance Research**: When studying how errors propagate through FHE computations

**When to Keep Disabled (Default):**
- **Production Systems**: Any real-world deployment
- **Correctness-Critical Applications**: When you need accurate results
- **Automated Testing**: When test pass/fail should depend on result correctness
- **Any scenario where incorrect results could cause problems**

### Important Warnings

**CRITICAL**: This modification allows **INCORRECT AND UNRELIABLE** results to be returned. The decrypted values will be corrupted when approximation error is too high.

**DO NOT USE** this in:
- Production code
- Final validation testing
- Any situation where you rely on result correctness
- Published benchmarks or performance comparisons

**ONLY USE** for:
- Debugging FHE parameter selection
- Understanding failure modes
- Development and experimentation
- Situations where you need to see "what would it look like" even if wrong

The implementation includes:
- Prominent warning messages to stderr indicating results are unreliable
- Specific error metrics (log standard deviation vs threshold) for each failure
- Clear indication this is Niobium-specific via `NB_` prefix
- Warnings that explicitly state "RESULTS ARE UNRELIABLE"

---


