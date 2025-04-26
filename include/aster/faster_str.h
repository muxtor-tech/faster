#ifndef FASTER_STR_H
#define FASTER_STR_H

#if FASTER_UNICODE_SUPPORT == FASTER_UNICODE_SUPPORT_AUTODETECT
#undef FASTER_UNICODE_SUPPORT

// Auto-detect based on platform and C23 capabilities
#if (defined(__STDC_HOSTED__) && __STDC_HOSTED__ == 0) // Embedded-like systems
#if defined(__STDC_UTF_8__) && __STDC_VERSION__ >= 202311L
#define FASTER_UNICODE_SUPPORT FASTER_UNICODE_SUPPORT_ONE_BYTE
#else
#define FASTER_UNICODE_SUPPORT FASTER_UNICODE_SUPPORT_NONE
#endif
#else                       // Hosted systems
#if INTPTR_MAX == INT32_MAX // 32-bit platforms
#if defined(__STDC_UTF_16__)
#define FASTER_UNICODE_SUPPORT FASTER_UNICODE_SUPPORT_TWO_BYTE
#elif defined(__STDC_UTF_32__) // Secondary fallback
#define FASTER_UNICODE_SUPPORT FASTER_UNICODE_SUPPORT_FOUR_BYTE
#else
#define FASTER_UNICODE_SUPPORT FASTER_UNICODE_SUPPORT_NONE
#endif
#elif INTPTR_MAX == INT64_MAX // 64-bit platforms
#if defined(__STDC_UTF_32__)
#define FASTER_UNICODE_SUPPORT FASTER_UNICODE_SUPPORT_FOUR_BYTE
#elif defined(__STDC_UTF_16__) // Secondary fallback
#define FASTER_UNICODE_SUPPORT FASTER_UNICODE_SUPPORT_TWO_BYTE
#else
#define FASTER_UNICODE_SUPPORT FASTER_UNICODE_SUPPORT_NONE
#endif
#else // Unknown architecture
#define FASTER_UNICODE_SUPPORT FASTER_UNICODE_SUPPORT_NONE
#endif
#endif

#endif

// we don't need the c++ compatibility warning
// the complaint about charx_t is a false positive
#define FASTER_UNICODE_SUPPORT_INVALID_CHARACTER_STRING_UTF8 "\xef\xbf\xbd"
#define FASTER_UNICODE_SUPPORT_INVALID_CHARACTER_VALUE 0xFFFD

#if FASTER_UNICODE_SUPPORT == FASTER_UNICODE_SUPPORT_ONE_BYTE
#include <uchar.h>
#define FASTER_UNICODE_MB_TO_UC_FUNC mbrtoc8
#define FASTER_UNICODE_UC_TO_MB_FUNC c8rtomb
#define ASTER_TEXT(s) u8##s
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wc++-compat"
typedef char8_t fchar_t;
#pragma GCC diagnostic pop
static_assert(sizeof(fchar_t) == 1, "unicode 8bit support not possible, fchar_t must be at least 1 byte");
#elif FASTER_UNICODE_SUPPORT == FASTER_UNICODE_SUPPORT_TWO_BYTE
#include <uchar.h>
#define FASTER_UNICODE_MB_TO_UC_FUNC mbrtoc16
#define FASTER_UNICODE_UC_TO_MB_FUNC c16rtomb
#define ASTER_TEXT(s) u##s
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wc++-compat"
typedef char16_t fchar_t;
#pragma GCC diagnostic pop
static_assert(sizeof(fchar_t) >= 2, "unicode 16bit support not possible, fchar_t must be at least 2 bytes");
#elif FASTER_UNICODE_SUPPORT == FASTER_UNICODE_SUPPORT_FOUR_BYTE
#include <uchar.h>
#define FASTER_UNICODE_MB_TO_UC_FUNC mbrtoc32
#define FASTER_UNICODE_UC_TO_MB_FUNC c32rtomb
#define ASTER_TEXT(s) U##s
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wc++-compat"
typedef char32_t fchar_t;
#pragma GCC diagnostic pop
static_assert(sizeof(fchar_t) >= 4, "unicode 32bit support not possible, fchar_t must be at least 4 bytes");
#else // no unicode support
#define ASTER_TEXT(s) s
#define faster_sprintf(s, mem, f, ...) sprintf(s, ASTER_TEXT(f), __VA_ARGS__)
typedef char fchar_t;
static_assert(sizeof(fchar_t) == 1, "wider char available, consider a switch to unicode support");
#endif

#define FASTER_STRING_MEMORY_SIZE(strlen) (sizeof(fchar_t) * strlen)
#define FASTER_DECLARE_RAW_STR(name, str) static const fchar_t name[] = ASTER_TEXT(str)

#define FASTER_DECLARE_FASTER_STR(name, str)                                                                                       \
  FASTER_DECLARE_RAW_STR(_##name##_rawstr, str);                                                                                   \
  static const faster_str_t name = {_##name##_rawstr, sizeof(_##name##_rawstr) / sizeof(fchar_t)}

#endif // FASTER_STR_H