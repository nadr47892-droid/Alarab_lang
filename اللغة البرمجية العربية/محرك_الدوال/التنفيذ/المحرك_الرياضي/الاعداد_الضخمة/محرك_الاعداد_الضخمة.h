#ifndef BIGINT_H
#define BIGINT_H

#define BIGINT_MAX 1024

typedef struct {
    int digits[BIGINT_MAX];
    int length;
} BigInt;

void bigint_from_string(BigInt *a, const char *str);
void bigint_print(const BigInt *a);
void bigint_add(BigInt *res, const BigInt *a, const BigInt *b);
void bigint_sub(BigInt *res, const BigInt *a, const BigInt *b);
void bigint_mul(BigInt *res, const BigInt *a, const BigInt *b);

#endif
