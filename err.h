/* 
    LICENSED UNDER MIT, see LICENSE for more detail.
    Copyright @ 2018 Braedon Wooding
*/

/*
    Even though compilers can make this smaller we want multiple error types
    to work together reasonably well so we need this!
*/
#define ERR_SIZE int

typedef enum _result_variant_t {
    ERROR,
    VALUE,
} result_variant;

typedef struct _result_t {
    union {
        void *ok;
        ERR_SIZE error;
    };
    result_variant variant;
} Result;

#define STRINGIFY(x) case x: return #x;

#define CONCATENATE(arg1, arg2)   CONCATENATE1(arg1, arg2)
#define CONCATENATE1(arg1, arg2)  CONCATENATE2(arg1, arg2)
#define CONCATENATE2(arg1, arg2)  arg1##arg2

#define FOR_EACH_1(what, x)         \
    what(x)

#define FOR_EACH_2(what, x, ...)    \
    what(x);                        \
    FOR_EACH_1(what, __VA_ARGS__);

#define FOR_EACH_3(what, x, ...)    \
    what(x);                        \
    FOR_EACH_2(what, __VA_ARGS__);

#define FOR_EACH_4(what, x, ...)    \
    what(x);                        \
    FOR_EACH_3(what,  __VA_ARGS__);

#define FOR_EACH_5(what, x, ...)    \
    what(x);                        \
    FOR_EACH_4(what,  __VA_ARGS__);

#define FOR_EACH_6(what, x, ...)    \
  what(x);                          \
  FOR_EACH_5(what,  __VA_ARGS__);

#define FOR_EACH_7(what, x, ...)    \
    what(x);                        \
    FOR_EACH_6(what,  __VA_ARGS__);

#define FOR_EACH_8(what, x, ...)    \
    what(x);                        \
    FOR_EACH_7(what,  __VA_ARGS__);

#define FOR_EACH_NARG(...) FOR_EACH_NARG_(__VA_ARGS__, FOR_EACH_RSEQ_N())
#define FOR_EACH_NARG_(...) FOR_EACH_ARG_N(__VA_ARGS__) 
#define FOR_EACH_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, N, ...) N 
#define FOR_EACH_RSEQ_N() 8, 7, 6, 5, 4, 3, 2, 1, 0

#define FOR_EACH_(N, what, ...) CONCATENATE(FOR_EACH_, N)(what, __VA_ARGS__)
#define FOR_EACH(what, ...) FOR_EACH_(FOR_EACH_NARG(__VA_ARGS__), what, __VA_ARGS__)

#define ERR_ENUM_GET_NAME(name, ...)            \
    char * name##_to_string(name val) {             \
        switch(val) {                               \
            FOR_EACH(STRINGIFY, __VA_ARGS__)     \
            default: return NULL;                   \
        }                                           \
    }

#define ERR_ENUM(name, ...) \
    typedef enum _##name##_t { __VA_ARGS__ } name; \
    ERR_ENUM_GET_NAME(name,__VA_ARGS__)

#define ERR(err) (Result){ .error = err, .variant = ERROR }

#define OK(x) (Result){ .ok = (void*)(unsigned long)x, .variant = VALUE }

#define IS_ERR(x) (x.variant == ERROR)

#define IS_OK(x) (x.variant == VALUE)

#define UNWRAP(x, type) (type)(unsigned long)x.ok

#define IF_LET(x, type, out) type out = UNWRAP(x, type); if (IS_OK(x))

#define TRY(res, type, out)                             \
    type out;                                           \
{                                                       \
    Result result = res;                                \
    if (IS_ERR(result)) return ERR(result.error);       \
    out = UNWRAP(result, type);                         \
}
