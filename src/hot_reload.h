#pragma once

#include <stdbool.h>

#include "ttt.h"

#ifdef HOT_RELOAD
    #define FUNC(name, ...) extern name##_t *name;
    LIST_OF_FUNCS
    #undef FUNC
    bool reload_libplug(void);
#else
    #define FUNC(name, ...) name##_t name;
    LIST_OF_FUNCS
    #undef FUNC
    #define reload_libplug() true
#endif  // HOT_RELOAD
