#ifndef BOX_UTIL_H
#define BOX_UTIL_H

#include <stdarg.h>
#include <stdio.h>

// 留下接口，方便将来修改malloc和free接口
static void* (*__box_malloc)(size_t) = malloc;
static void (*__box_free)(void*) = free;

#define box_malloc  __box_malloc
#define box_free        __box_free

// 日志
// printf(), perror()
static int (*__box_printf)(const char* fmt, ...) = printf;
static int(*__box_vsprintf)(const char* fmt, va_list ap) = vprintf;

static inline void __box_log(char* file, int line, const char* fmt, ...)
{
    __box_printf("%s, %d: ", file, line);

    va_list ap;
    va_start(ap, fmt);
    __box_vsprintf(fmt, ap);
    va_end(ap);
}

#define box_log(fmt, ...) __box_log(__FILE__, __LINE__, fmt,  ##__VA_ARGS__)

/*
    box_malloc  --> malloc
    box_free  --> free
    box_log  --> printf
*/


#endif // BOX_UTIL_H
