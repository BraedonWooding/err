#include "err.h"
#include <stdio.h>
#include <string.h>

ERR_ENUM(args_error, INVALID_ARGS)
ERR_ENUM(parse_int_error, INT_INVALID_CHAR, INT_ARITHMETIC_OVERFLOW)
ERR_ENUM(parse_bool_error, BOOL_INVALID_CHARS)
ERR_ENUM_GROUP(err_to_string, args_error, parse_int_error, parse_bool_error)

Result parse_int(char *str, int len) {
    int res = 0;
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

WRAP_MAIN(actual_main, err_to_string) {
    // Note: you only get access to argc/argv if you want envp but that is NOT portable
    // But `WRAP_MAIN_ENVP` will add it
    GUARD(argc == 2, INVALID_ARGS);
    int out = TRY(parse_int(argv[1], strlen(argv[1])), int);
    printf("Success parsed int %d\n", out);
    return OK(0);
}
