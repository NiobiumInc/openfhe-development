# Niobium-Specific OpenFHE Modifications

This document describes the Niobium-specific changes made to the OpenFHE library fork.

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

.
