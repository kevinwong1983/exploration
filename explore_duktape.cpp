#include "boost/variant.hpp"
#include <iostream>
#include <gtest/gtest.h>
#include <boost/asio.hpp>
#include <time.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/system/system_error.hpp>
#include "src/duktape.h"

static duk_ret_t native_print(duk_context *ctx) {
    duk_push_string(ctx, " ");
    duk_insert(ctx, 0);
    duk_join(ctx, duk_get_top(ctx) - 1);
    printf("%s\n", duk_safe_to_string(ctx, -1));
    return 0;
}

static duk_ret_t native_adder(duk_context *ctx) {
    int i;
    int n = duk_get_top(ctx);  /* #args */
    double res = 0.0;

    for (i = 0; i < n; i++) {
        res += duk_to_number(ctx, i);
    }

    duk_push_number(ctx, res);
    return 1;  /* one return value */
}

/* For brevity assumes a maximum file length of 16kB. */
static void push_file_as_string(duk_context *ctx, const char *filename) {
    FILE *f;
    size_t len;
    char buf[16384];

    f = fopen(filename, "rb");
    if (f) {
        len = fread((void *) buf, 1, sizeof(buf), f);
        fclose(f);
        duk_push_lstring(ctx, (const char *) buf, (duk_size_t) len);
    } else {
        duk_push_undefined(ctx);
    }
}

static void debug(duk_context *ctx){
    duk_push_context_dump(ctx);
    printf("\n\n%s\n\n", duk_to_string(ctx, -1));
    duk_pop(ctx);
}

TEST(bind, duktape) {
    duk_context *ctx = duk_create_heap_default();
    duk_push_c_function(ctx, native_print, DUK_VARARGS);
    duk_put_global_string(ctx, "print");
    duk_push_c_function(ctx, native_adder, DUK_VARARGS);
    duk_put_global_string(ctx, "adder");
    push_file_as_string(ctx, "/Users/user/CLionProjects/experimental/myscript.js");
    debug(ctx);

    duk_eval(ctx);
    debug(ctx);
//    duk_eval_string(ctx, "print('Hello world!');");
//    duk_eval_string(ctx, "print('2+3=' + adder(2, 3, 4, 5, 6, 7));");

    duk_pop(ctx);  /* pop eval result */
    debug(ctx);
    duk_destroy_heap(ctx);
}
