#include "err.h"

/* None of these headers are required for anything but the examples to show off things */
#include <stdbool.h> // for parse_bool
#include <string.h> // for strcmp
#include <stdio.h> // for printf
#include <stdlib.h> // for the malloc example

/* Generate the error types */
ERR_ENUM(parse_int_error, INT_INVALID_CHAR, INT_ARITHMETIC_OVERFLOW)
ERR_ENUM(parse_bool_error, BOOL_INVALID_CHARS)

Result parse_int(char *str, int len) {
    long res = 0;
    for (char *c = str; *c != '\0'; c++) {
        if (*c >= '0' && *c <= '9') {
            if (res >= (__INT_MAX__ - *c - '0')/10) return ERR(INT_ARITHMETIC_OVERFLOW);
            res *= 10;
            res += *c - '0';
        } else {
            return ERR(INT_INVALID_CHAR);
        }
    }
    return OK(res);
}

Result parse_bool(char *str, int len) {
    bool res = 0;
    if (!strcmp(str, "true")) return OK(true);
    else if (!strcmp(str, "false")) return OK(false);
    else return ERR(BOOL_INVALID_CHARS);
    return OK(res);
}

typedef struct my_val_t {
    int x;
    int y;
    double z;
} *MyVal;

ERR_ENUM(oom, OutOfMemory);

Result get_myval(int x, int y, double z) {
    MyVal out = malloc(sizeof(struct my_val_t));
    GUARD(out != NULL, OutOfMemory);
    out->x = x;
    out->y = y;
    out->z = z;
    return OK(out);
}

Result get_float(float x) {
    return OK(x);
}

Result try_example(char *str, int len) {
    int out = TRY(parse_int(str, len), int);
    return OK(out);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Parse parse arg for int\n");
        abort();
    }

    Result x = parse_int(argv[1], strlen(argv[1]));
    // you can do it this long way
    if (IS_ERR(x)) {
        // it will generate a `x_to_string` for each error enum
        // can just grab error like `.error`
        printf("Is error %s\n", parse_int_error_to_string(x.error));
    } else {
        // to access the data
        int unwrapped = UNWRAP(x, int);
        printf("Is result: %d\n", unwrapped);
    }

    // you can also use this shorter way
    IF_LET(x, int, out_int) {
        printf("Same result: %d\n", out_int);
    } else {
        printf("Still error: %s\n", parse_int_error_to_string(x.error));
    }

    // works with other stuff too
    // like this boolean
    Result y = parse_bool("true", 4);
    printf("True is %d (which is true)\n", UNWRAP(y, bool));

    // you can also just skip the 'business' of grabbing the result and just get the value
    MyVal val = UNWRAP(get_myval(1, 2, 5.983383838), MyVal);
    printf("MyVal: x: %d, y: %d, z: %lf\n", val->x, val->y, val->z);

    int k = UNWRAP(try_example(argv[1], strlen(argv[1])), int);
    printf("Try is still %d\n", k);

    printf("FLT: %f\n", UNWRAP(get_float(5.39), float));
}
