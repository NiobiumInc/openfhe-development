#ifndef OPENFHE_CPROBES_H
#define OPENFHE_CPROBES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void openfhe_cprobe_execute();

void openfhe_cprobe_annotate(const char*);

void openfhe_cprobe_id(uintptr_t);
uintptr_t* openfhe_cprobe_address(uintptr_t);
uintptr_t* openfhe_cprobe_result(uintptr_t);

void openfhe_cprobe_discrete_gaussian(uintptr_t, int);
void openfhe_cprobe_discrete_uniform(uintptr_t, int);
void openfhe_cprobe_binary_uniform(uintptr_t, int);
void openfhe_cprobe_ternary_uniform(uintptr_t, int);

void openfhe_cprobe_precompute(uintptr_t, int);

void openfhe_cprobe_zero(uintptr_t, int);
void openfhe_cprobe_max(uintptr_t, int);

void openfhe_cprobe_input(uintptr_t, int);
void openfhe_cprobe_output(uintptr_t, int);

void openfhe_cprobe_key(uintptr_t, int);

void openfhe_cprobe_copy(uintptr_t, uintptr_t);
void openfhe_cprobe_move(uintptr_t, uintptr_t);

void openfhe_cprobe_add(uintptr_t, uintptr_t, uintptr_t, uint64_t);
void openfhe_cprobe_sub(uintptr_t, uintptr_t, uintptr_t, uint64_t);
void openfhe_cprobe_mul(uintptr_t, uintptr_t, uintptr_t, uint64_t);

void openfhe_cprobe_addi(uintptr_t, uintptr_t, uint64_t, uint64_t);
void openfhe_cprobe_subi(uintptr_t, uintptr_t, uint64_t, uint64_t);
void openfhe_cprobe_muli(uintptr_t, uintptr_t, uint64_t, uint64_t);

void openfhe_cprobe_automorphism(uintptr_t, uintptr_t, uint64_t, uint64_t, uint64_t, uint64_t);

void openfhe_cprobe_switchmodulus(uintptr_t, uintptr_t, uint64_t, uint64_t);

void openfhe_cprobe_ntt(uintptr_t, uintptr_t, uint64_t, uint64_t);
void openfhe_cprobe_intt(uintptr_t, uintptr_t, uint64_t, uint64_t);

#ifdef __cplusplus
}
#endif

#endif
