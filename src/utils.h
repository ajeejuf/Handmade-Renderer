
#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <math.h>
#include <ctype.h>
#include <time.h>

#define HMM_ANGLE_USER_TO_INTERNAL
#define HANDMADE_MATH_USE_DEGREES
#include <HandmadeMath.h>

#define internal static
#define global static
#define local static

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t b8;
typedef int32_t b32;

typedef float f32;
typedef double f64;

typedef HMM_Vec2 v2;
typedef HMM_Vec3 v3;
typedef HMM_Vec4 v4;
typedef HMM_Mat2 mat2;
typedef HMM_Mat3 mat3;
typedef HMM_Mat4 mat4;
typedef HMM_Quat quat;

#define MAX_PATH 260

#define ARRAY_COUNT(a) (sizeof((a))/sizeof((a)[0]))

#ifdef __EMSCRIPTEN__

#define ASSERT(c) \
do { \
if(!(c)) { \
emscripten_force_exit(EXIT_FAILURE); \
} \
} while(0)

#define ASSERT_LOG(c, f, ...) \
do { \
if(!(c)) { \
fprintf(stderr, "Assertion failed at line %d: ", __LINE__); \
fprintf(stderr, f, ##__VA_ARGS__); \
fprintf(stderr, "\n"); \
emscripten_force_exit(EXIT_FAILURE);; \
} \
} while(0)

#define LOG(format, ...) \
do { \
emscripten_log(EM_LOG_CONSOLE, format, ##__VA_ARGS__); \
} while(0)

#else

#define ASSERT(c) \
do { \
if(!(c)) { \
exit(EXIT_FAILURE); \
} \
} while(0)

#define ASSERT_LOG(c, f, ...) \
do { \
if(!(c)) { \
fprintf(stderr, "Assertion failed at line %d: ", __LINE__); \
fprintf(stderr, f, ##__VA_ARGS__); \
fprintf(stderr, "\n"); \
exit(EXIT_FAILURE); \
} \
} while(0)

#define LOG(format, ...) \
do { \
fprintf(stdout, format, ##__VA_ARGS__); \
fprintf(stdout, "\n"); \
} while(0)

#endif


#define BYTES(x) (x)
#define KILOBYTES(x) (x << 10)
#define MEGABYTES(x) (x << 20)
#define GIGABYTES(x) (((u64)x) << 30)


#include "stack.h"


// NOTE(ajeej): Memory Arena

typedef struct memory_arena_t {
    u64 cursor;
    u64 max;
    void *mem;
} memory_arena_t;

internal void
init_memory_arena(memory_arena_t *arena, void *mem, u64 size) {
    memset(arena, 0, sizeof(memory_arena_t));
    memset(mem, 0, size);
    
    arena->max = size;
    arena->mem = mem;
}

#define arena_push_struct(a, s) arena_push(a, sizeof(s))
#define arena_push_array(a, s, c) arena_push(a, sizeof(s)*c)

internal void *
arena_push(memory_arena_t *arena, u64 size)
{
    ASSERT_LOG(arena->cursor+size <= arena->max, "Area is FULL.");
    
    void *m = (void *)((u8 *)arena->mem + arena->cursor);
    arena->cursor += size;
    return m;
}

internal u64
arena_get_remaining(memory_arena_t *arena)
{
    return arena->max-arena->cursor;
}


// NOTE(ajeej): String

internal char *
cstr_cat(char *dst, char *src)
{
    while (*dst) dst++;
    while ((*dst++ = *src++));
    return --dst;
}

#define cstr_cat_many(d, ...)\
__cstr_cat_many(d, __VA_ARGS__, (char *)0)

internal void
__cstr_cat_many(char *dst, ...) 
{
    va_list ap;
    char *str, *iter = dst;
    
    va_start(ap, dst);
    for (str = va_arg(ap, char *);
         str;
         str = va_arg(ap, char *))
        iter = cstr_cat(iter, str);
    va_end(ap);
}

char *insert_string_at_index(const char *o, u32 idx, const char *n)
{
    size_t o_len = strlen(o);
    size_t n_len = strlen(n);
    
    if (idx < 0 || idx > o_len)
        return NULL;
    
    char *str = malloc(o_len+n_len+1);
    
    strncpy(str, o, idx);
    strncpy(str+idx, n, n_len);
    strcpy(str+idx+n_len, o+idx);
    
    return str;
}

internal char *
cstr_dup(char *src)
{
    u64 len = strlen(src);
    char *res = malloc(len+1);
    memcpy(res, src, len);
    res[len] = 0;
    
    return res;
}

internal void
cstr_to_upper(char *dst, const char *str)
{
    while(*str)
        *(dst++) = toupper(*(str++));
    *dst = 0;
}

internal char *
cstr_make(const char **strs, u32 count)
{
    va_list ap;
    char *start, *res;
    size_t len = 0, s_len = 0;
    
    int i;
    for (i = 0; i < count; i++)
        len += strlen(strs[i]);
    
    start = res = malloc(len+1);
    
    for (i = 0; i < count; i++) {
        s_len = strlen(strs[i]);
        memcpy(start, strs[i], s_len);
        start += s_len;
    }
    
    res[len] = 0;
    
    return res;
}


// NOTE(ajeej): Bits

internal void
set_bit(u8 *ar, u64 bit_idx)
{
    u64 byte_idx = bit_idx / 8;
    u8 bit_pos = bit_idx % 8;
    
    ar[byte_idx] |= (1 << bit_pos);
}

internal u8
check_bit(u8 *ar, u64 bit_idx)
{
    u64 byte_idx = bit_idx / 8;
    u8 bit_pos = bit_idx % 8;
    
    return (ar[byte_idx] & (1 << bit_pos)) != 0;
}

// NOTE(ajeej): only works with powers of 2
internal u64
align_offset(u64 offset, u64 alignment)
{
    return (offset + alignment - 1) & ~(alignment - 1);
}


// NOTE(ajeej): File IO

internal char *
get_build_dir(char *execute_path)
{
    char *slash = strrchr(execute_path, '/'), *res;
    if (slash == NULL)
        return NULL;
    
    size_t len = slash - execute_path;
    res = malloc(len+1);
    memcpy(res, execute_path, len);
    res[len] = 0;
    return res;
}

internal u32
get_extension(char *ext, char *path)
{
    char *period = strrchr(path, '.');
    
    if (period == NULL)
        return 0;
    
    u32 len = strlen(period);
    memcpy(ext, period+1, len);
    
    return 1;
}

internal char *
read_file(const char *fn, size_t *o_size)
{
    FILE *file = fopen(fn, "r");
    ASSERT_LOG(file != NULL, "Couldn't open file %s", fn);
    
    size_t size;
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char *data = (char *)malloc(size+1);
    fread(data, 1, size, file);
    data[size] = '\0';
    
    fclose(file);
    
    if (o_size)
        *o_size = size;
    
    return data;
}

internal i32
copy_file(const char *src, const char *dst)
{
    FILE *src_file, *dst_file;
    char buffer[1024];
    size_t bytes_read;
    
    src_file = fopen(src, "rb");
    if (src_file == NULL) return 1;
    
    dst_file = fopen(dst, "wb");
    if (dst_file == NULL) { 
        fclose(src_file);
        return 1;
    }
    
    while((bytes_read = fread(buffer, 1, sizeof(buffer), src_file)) > 0) {
        size_t bytes_written = fwrite(buffer, 1, bytes_read, dst_file);
        if (bytes_written != bytes_read) {
            fclose(src_file);
            fclose(dst_file);
            return 1;
        }
    }
    
    if (ferror(src_file)) {
        fclose(src_file);
        fclose(dst_file);
        return 1;
    }
    
    fclose(src_file);
    fclose(dst_file);
    return 0;
}

#endif //UTILS_H
