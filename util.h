#ifndef __UTIL_H__
#define __UTIL_H__
#include <stdint.h>

typedef uint8_t byte;
typedef uint16_t half;
typedef uint32_t word;
typedef uint64_t dword;

typedef int8_t sbyte;
typedef int16_t shalf;
typedef int32_t sword;
typedef int64_t sdword;

#define popcount(x) __builtin_popcountll(x)
#define FAKELITTLE_HALF(h) ((((h) >> 8u) & 0xFFu) | (((h) << 8u) & 0xFF00u))
#define FAKELITTLE_WORD(w) (FAKELITTLE_HALF((w) >> 16u) | (FAKELITTLE_HALF((w) & 0xFFFFu)) << 16u)

#define INLINE static inline __attribute__((always_inline))
#define PACKED __attribute__((__packed__))

#define unlikely(exp) __builtin_expect(exp, 0)
#define likely(exp) __builtin_expect(exp, 1)

#define N64_APP_NAME "dgb n64"

//#define ASSERTWORD(type) static_assert(sizeof(type) == sizeof(word), #type " must be 32 bits");
//#define ASSERTDWORD(type) static_assert(sizeof(type) == sizeof(dword), #type " must be 64 bits");

#define ASSERTWORD(type)
#define ASSERTDWORD(type)

#define BYTE_ADDRESS(addr) (addr)

#define logfatal(x, ...)
#define logwarn(x, ...)
#define logdebug(x, ...)
#define loginfo(x, ...)
#define logtrace(x, ...)
#define unimplemented(x, y, ...)

#endif
