#include "threads/float_arith.h"

int i_sub_f(int i, int f) {
    return i*FRACTION - f;
}

int i_mul_f(int i, int f) {
    return i * f;
}

int f_add_i(int f, int i) {
    return f + i * FRACTION;
}

int f_sub_i(int f, int i) {
    return f_add_i(f, -i);
}

int f_mul_f(int f1, int f2) {
    int64_t result = f1;
    result = result * f2 / FRACTION;
    return (int)result;
}

int f_div_f(int f1, int f2) {
    int64_t result = f1;
    result = result * FRACTION / f2;
    return (int)result;
}

int f_add_f(int f1, int f2) {
    return f1 + f2;
}

int f_sub_f(int f1, int f2) {
    return f1 - f2;
}

int f_div_i(int f, int i) {
    return f / i;
}
