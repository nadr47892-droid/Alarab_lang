#include <stdio.h>
#include <string.h>
#include "محرك_الاعداد_الضخمة.h"

void bigint_from_string(BigInt *a, const char *str)
{
    a->length = strlen(str);

    for (int i = 0; i < a->length; i++)
        a->digits[i] = str[a->length - 1 - i] - '0';
}

void bigint_print(const BigInt *a)
{
    for (int i = a->length - 1; i >= 0; i--)
        printf("%d", a->digits[i]);
    printf("\n");
}

void bigint_add(BigInt *res, const BigInt *a, const BigInt *b)
{
    int carry = 0;
    int max = (a->length > b->length) ? a->length : b->length;

    for (int i = 0; i < max; i++) {
        int sum = carry;

        if (i < a->length) sum += a->digits[i];
        if (i < b->length) sum += b->digits[i];

        res->digits[i] = sum % 10;
        carry = sum / 10;
    }

    res->length = max;

    if (carry) {
        res->digits[max] = carry;
        res->length++;
    }
}

void bigint_mul(BigInt *res, const BigInt *a, const BigInt *b)
{
    memset(res, 0, sizeof(BigInt));

    for (int i = 0; i < a->length; i++) {
        int carry = 0;

        for (int j = 0; j < b->length; j++) {

            int temp = res->digits[i + j] +
                       a->digits[i] * b->digits[j] +
                       carry;

            res->digits[i + j] = temp % 10;
            carry = temp / 10;
        }

        res->digits[i + b->length] += carry;
    }

    res->length = a->length + b->length;

    while (res->length > 1 &&
           res->digits[res->length - 1] == 0)
        res->length--;
}
