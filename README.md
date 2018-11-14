# Err - Error Handling in C

Just a nice small error handling library in C.  Highly based off the rust result type.

> Note: is not strictly C11 and utilises extensions, only works on gcc and clang.

> You just need `err.h`.

## Example

```c
#include "err.h"

/* None of these headers are required for anything but the examples to show off things */
// Except for stdio if you are going to use wrap_main (as we need fprintf)
#include <stdbool.h> // for parse_bool
#include <string.h> // for strcmp
#include <stdio.h> // for printf
#include <stdlib.h> // for the malloc example

/* Generate the error types */
// All error cases get a unqiue ID, and so you can use them together no problem.
// You have to give it a name more for your use than ours.
ERR_ENUM(parse_int_error, INT_INVALID_CHAR, INT_ARITHMETIC_OVERFLOW)
ERR_ENUM(parse_bool_error, BOOL_INVALID_CHARS)

// Result is the new return type, it indicates that EITHER it returns ERR or OK.
Result parse_int(char *str, int len) {
    // no changes to your function till...
    int res = 0;
    for (char *c = str; *c != '\0'; c++) {
        if (*c >= '0' && *c <= '9') {
            // you need to return, then just 'wrap' your return in ERR(...) or OK(...)
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

// You can see how it can be quite useful!
Result get_myval(int x, int y, double z) {
    MyVal out = malloc(sizeof(struct my_val_t));
    if (out == NULL) return ERR(OutOfMemory);
    out->x = x;
    out->y = y;
    out->z = z;
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
    MyVal val = UNWRAP(get_myval(1, 2, 5.0), MyVal);
    printf("MyVal: x: %d, y: %d, z: %lf\n", val->x, val->y, val->z);
}
```

## Try

A common pattern is

```c
Result foo(int bar) {
    Result res = do_x(bar);
    if (IS_ERR(res)) return ERR(res.error);
    int out = UNWRAP(res, int);

    // continue on
}
```

This is so common that there is a new macro for it;

```c
Result foo(int bar) {
    int out = TRY(do_x(bar), int);

    // continue on
}
```

## Other stuff

- `GUARD(cond, else_err)` if condition is false returns error.
- `UNIQUE_ERRORS` is a #define in err.h if enabled (by default is) then each error gets a unique ID regardless of where it is located.
  - This means that you can use errors from different `ERR_ENUM`s together without a problem.

But you may ask, but what if I want to wrap main?  Such that I can propagate (`TRY`) errors until I hit main at which point it prints the error out then aborts?  Well then I say to you, use `WRAP_MAIN(args...)`;

```c
// presuming parse_int and parse_int_error is defined
ERR_ENUM(args_error, INVALID_ARGS)
// this groups them under a single function
ERR_ENUM_GROUP(err_to_string, args_error, parse_int_error)

// you have to pass the name of the function that you will write code in (in this case actual_main)
// and then the name of the error handler to use.
WRAP_MAIN(actual_main, err_to_string) {
    // Note: you only get access to argc/argv if you want envp but that is NOT portable
    // But `WRAP_MAIN_ENVP` will add it
    GUARD(argc != 2, INVALID_ARGS);
    int out = TRY(parse_int(argv[1], strlen(argv[1])), int);
    printf("Success %d\n", out);
    return OK(0);
}
```

There is also one called `WRAP_MAIN_ENVP` incase you want the non-portable `envp` parameter.

## What can't this libary do

- Typesafety is the biggest concern, since there are no ways for typesafety to be properly implemented in C there is nothing stopping you from converting the result to any type you want!
  - In actual fact this really isn't that much of a concern if UNIQUE_ERRORS is enabled since 
- Slight 'note' IF_LET does leak it's variable outside it's scope (only way to implement such as to look like an if statement), shouldn't really be a concern and you can always do `{ IF_LET(cond, int, out) { /* ... */ } }` if you need to remove the scoping.
- Run on MSVC; it only supports clang and gcc

## How it is implemented

Extremely similar to this;

```c
struct result {
    union {
        int error;
        void *data;
    };
    enum {
        ERROR,
        OK,
    } variant;
}
```

So as you can see if your data is `8 bytes` long then you only have to pay the cost for the enum, which is only a byte cost.  However you may pay up to 8 bytes if your data is only a single byte.  It is reasonably small cost, and in the future I may implement 'results' that are smaller such as `ShortResult` (data is 4 bytes so no pointers) 
